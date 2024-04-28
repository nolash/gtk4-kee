import os
import sys
import io
import logging
import hashlib
from Crypto.Cipher import ChaCha20_Poly1305
from Crypto.PublicKey import ECC
import Crypto.IO.PKCS8
import Crypto.Util.asn1
from Crypto.Signature import eddsa
import lmdb
import time
import datetime
import shutil
import email.message
import random
from faker import Faker
from faker.providers import lorem
import varint
from pyasn1.codec.der.encoder import encode as der_encode
from pyasn1.codec.der.decoder import decode as der_decode

from testdata_asn1schema import KeeEntryHead
from testdata_asn1schema import KeeEntry

logging.basicConfig(level=logging.DEBUG)
logg = logging.getLogger()

fake = Faker()
fake.add_provider(lorem)

FLAGS_SIGNER_IS_BOB = 1 << 0

NOBODY = b'\x00' * 64
NOSIG = b''
PFX_LEDGER_HEAD = b'\x01'
PFX_LEDGER_ENTRY = b'\x02'
PFX_LEDGER_PUBKEY = b'\x03'
PFX_LEDGER_CACHE_SUMS = b'\x80'

random.seed(int(time.time_ns()))


def padbytes(b, padsize=4096):
    l = padsize - (len(b) % padsize)
    b += os.urandom(l)
    return b


def db_init(d):
    d = os.path.join(d, 'testdata_mdb')
    logg.info('using d for db' + d)

    try:
        shutil.rmtree(d)
    except FileNotFoundError:
        pass
    os.makedirs(d)
    return d 


class LedgerContent(email.message.EmailMessage):

    def __init__(self):
        super(LedgerContent, self).__init__()
        self.set_default_type("text/plain")
        self.add_header("Subject", fake.sentence())
        self.set_content(fake.paragraph())


    def kv(self):
        b = self.as_bytes()
        h = hashlib.new("sha512")
        h.update(b)
        z = h.digest()
        return (z, b,)


class LedgerHeadContent(LedgerContent):
    pass


class LedgerEntryContent(LedgerContent):
    pass


class LedgerSigner:

    def __init__(self, crypto_dir):
        self.signer = {}
        self.keypair = {}
        self.pubkey_rindex = {}
        self.names = {}
        self.crypto_dir = crypto_dir


    def get_pubkey(self, k):
        return self.keypair[k][1]


    def get_name(self, k):
        return self.names[k]


    def __write_key(self, keyname, outdir, pin):
        (pk, pubk) = self.keypair[keyname]
        wt = io.BytesIO()
        wt.write(b"(8:key-data(10:public-key(3:ecc(5:curve7:Ed25519)(1:q32:")
        wt.write(pubk)
        wt.write(b")))(11:private-key(3:ecc(5:curve7:Ed25519)(1:q32:")
        wt.write(pubk)
        wt.write(b")(1:d32:")
        wt.write(pk)
        wt.write(b"))))")
        b = wt.getvalue()
        fp = os.path.join(self.crypto_dir, keyname + '.key.sexp')
        w = open(fp, 'wb')
        w.write(b)
        w.close()

        l = len(b)
        bl = l.to_bytes(4, byteorder='little')
        h = hashlib.new('sha256')
        h.update(b'1234')
        z_pin = h.digest()
        nonce = os.urandom(12)
        cph = ChaCha20_Poly1305.new(key=z_pin, nonce=nonce)
        r = cph.encrypt(bl + b)
        r = padbytes(r)

        fp = os.path.join(self.crypto_dir, keyname + '.key.bin')
        w = open(fp, 'wb')
        w.write(nonce + r)
        w.close()

        wt = io.BytesIO()
        wt.write(b"(8:key-data(10:public-key(3:ecc(5:curve7:Ed25519)(1:q32:")
        wt.write(pubk)
        wt.write(b"))))")
        b = wt.getvalue()
        fp = os.path.join(self.crypto_dir, keyname + '.pubkey.sexp')
        w = open(fp, "wb")
        w.write(b)
        w.close()


    def create_key(self, keyname, outdir=None, pin='1234'):
        k = ECC.generate(curve='Ed25519')
        pk_pkcs8 = k.export_key(format='DER')
        pk_der = Crypto.IO.PKCS8.unwrap(pk_pkcs8)
        pk = Crypto.Util.asn1.DerOctetString().decode(pk_der[1], strict=True).payload
        pubk = k.public_key().export_key(format='raw')


        self.signer[keyname] = eddsa.new(k, 'rfc8032')
        self.keypair[keyname] = (pk, pubk)
        self.pubkey_rindex[pubk] = keyname

        self.__write_key(keyname, outdir, pin)
        
        self.names[keyname] = fake.name()

        return pubk


    def sign(self, keyname, msg):
        h = hashlib.sha512()
        h.update(msg)
        z = h.digest()

        fp = os.path.join(self.crypto_dir, z.hex())
        w = open(fp, 'wb')
        w.write(msg)
        w.close()
    
        b = self.signer[keyname].sign(z)

        logg.debug('signature for {}: {}'.format(z.hex(), b.hex()))

        return b


class Ledger:

    @classmethod
    def data_add(self, data_dir, k, v):
        fp = os.path.join(data_dir, k.hex())
        f = open(fp, 'wb')
        f.write(v)
        f.close()


class LedgerGenerator:

    credit_alice_min = 1000
    credit_alice_max = 10000
    credit_bob_min = 1000
    credit_bob_max = 10000

    def __init__(self):
        self.credit_alice = random.randint(self.credit_alice_min, self.credit_alice_max)
        self.credit_bob = random.randint(self.credit_bob_min, self.credit_bob_max)
        self.collateral_alice = random.randint(self.credit_alice_min, self.credit_alice_max)
        self.collateral_bob = random.randint(self.credit_bob_min, self.credit_bob_max)
        self.count = 0
        logg.debug('new generator with credit alice {} bob {}'.format(self.credit_alice, self.credit_bob))


    def delta(self, collateral=False, credit=True):
        delta_credit = 0
        delta_collateral = 0
   
        single_is_bob = False
        if self.count == 0:
            delta_credit = self.credit_alice
            delta_collateral = self.collateral_alice
        elif self.count == 1:
            delta_credit = self.credit_bob
            delta_collateral = self.collateral_bob
            single_is_bob = True
        else:
            if self.credit_bob == 0 and self.credit_alice == 0:
                raise OverflowError()

        if delta_credit == 0:
            single_is_bob = bool(random.randint(0, 1))
            if self.credit_bob == 0:
                single_is_bob = False
            elif self.credit_alice == 0:
                    single_is_bob = True

            if credit:
                if single_is_bob:
                    delta_credit = random.randint(1, self.credit_bob) * -1
                    self.credit_bob += delta_credit
                else:
                    delta_credit = random.randint(1, self.credit_alice) * -1
                    self.credit_alice += delta_credit

            if collateral:
                if single_is_bob:
                    delta_collateral = random.randint(1, self.collateral_bob) * -1
                    self.collateral_bob += delta_collateral
                else:
                    delta_collateral = random.randint(1, self.collateral_alice) * -1
                    self.collateral_alice += delta_collateral
          
        delta_credit_bob = 0
        delta_credit_alice = 0
        if single_is_bob:
            delta_credit_bob = delta_credit
        else:
            delta_credit_alice = delta_credit

        delta_collateral_bob = 0
        delta_collateral_alice = 0
        if single_is_bob:
            delta_collateral_bob = delta_collateral
        else:
            delta_collateral_alice = delta_collateral

        self.count += 1

        logg.debug('credit delta alice {} ({}) bob {} ({}) isbob {}'.format(delta_credit_alice, self.credit_alice, delta_credit_bob, self.credit_bob, single_is_bob))
        #logg.debug('collateral delta alice  {} ({}) bob {} ({})'.format(delta_collateral_alice, self.collateral_alice, delta_collateral_bob, self.collateral_bob))
        return (delta_credit, delta_collateral, single_is_bob,)


class LedgerHead(Ledger):

    def __init__(self, alice_key=None, bob_key=None, body=NOBODY):
        self.uoa = "USD"
        self.uoa_decimals = 2
        if alice_key == None:
            alice_key = os.urandom(65)
        self.alice_pubkey_ref = alice_key
        if bob_key == None:
            bob_key = os.urandom(65)
        self.bob_pubkey_ref = bob_key
        logg.info('new ledger header with alice {} bob {}'.format(self.alice_pubkey_ref.hex(), self.bob_pubkey_ref.hex()))
        self.body = LedgerHeadContent()


    def serialize(self, data_dir, w=sys.stdout.buffer):
        o = KeeEntryHead()
        o['uoa'] = self.uoa
        o['uoaDecimals'] = self.uoa_decimals
        o['alicePubKey'] = self.alice_pubkey_ref
        o['bobPubKey'] = self.bob_pubkey_ref
        (k, v) = self.body.kv()
        self.data_add(data_dir, k, v)
        o['body'] = k
        b = der_encode(o)
        w.write(b)


    @staticmethod
    def to_key(b):
        r = b''
        r += PFX_LEDGER_HEAD
        v = time.time_ns()
        b = v.to_bytes(8, byteorder='big')
        r += b

        return r


class LedgerEntry(Ledger):

    credit_alice = 0
    credit_bob = 0
    collateral_alice = 0
    collateral_bob = 0
    ms = 0

    def __init__(self, head, signer, generator, parent=None, body=NOBODY, bob_name='bob'):
        self.head = head
        self.parent = parent
        if self.parent == None:
            self.parent = b'\x00' * 64
        self.timestamp = time.time_ns()

        self.body = LedgerEntryContent()

        delta = generator.delta()
        self.signer_sequence = []
        if delta[2]:
            self.signer_sequence = [bob_name, 'alice']
        else:
            self.signer_sequence = ['alice', bob_name]
        self.credit_delta = delta[0]
        self.collateral_delta = delta[1]

        self.request_signature = NOSIG
        self.response_signature = NOSIG
        self.signer = signer


    def serialize(self, data_dir, w=sys.stdout.buffer):
        o = KeeEntry()
        o['parent'] = self.parent
        o['timestamp'] = self.timestamp
        o['creditDelta'] = self.credit_delta
        o['collateralDelta'] = self.collateral_delta


        (k, v) = self.body.kv()
        self.data_add(data_dir, k, v)
        o['body'] = k

        o['signatureRequest'] = self.request_signature
        o['response'] = False
        o['signatureResponse'] = self.response_signature

        logg.debug('encoding new entry for request signature: {}'.format(o))

        b = der_encode(o)
        self.request_signature = self.signer.sign(self.signer_sequence[0], self.head + b)
        o['signatureRequest'] = self.request_signature
        o['response'] = True

        b = der_encode(o)
        self.response_signature = self.signer.sign(self.signer_sequence[1], self.head + b)
        o['signatureResponse'] = self.response_signature

        b = der_encode(o)
        flag = b'\x00'
        if self.signer_sequence[0] != 'alice':
            flag = b'\x01'
        w.write(b + flag)

        LedgerEntry.ms += 1
        if LedgerEntry.ms > 999:
            LedgerEntry.ms = 0


    @staticmethod
    def to_key(v, k):
        r = b''
        r += PFX_LEDGER_ENTRY
        r += k
       
        o = der_decode(v, asn1Spec=KeeEntry())
        ts = o[0]['timestamp']
        tsb = int(ts).to_bytes(8, byteorder='big')
        #logg.debug('ts {} ({}): of {}'.format(ts, tsb, v.hex()))
        r += tsb
        return r
  

def generate_entry(data_dir, signer, generator, head, bob_name, parent):
    o = LedgerEntry(head, signer, generator, parent=parent, bob_name=bob_name)
    w = io.BytesIO()
    r = o.serialize(data_dir, w=w)
    h = hashlib.new('sha512')
    b = w.getvalue()
    h.update(b)
    z = h.digest()
    return (z, b,)


def generate_ledger(dbi, data_dir, signer, bob_name, entry_count=3, alice=None, bob=None):
    r = []
    o = LedgerHead(alice_key=alice, bob_key=bob)
    w = io.BytesIO()
    o.serialize(data_dir, w=w)
    h = hashlib.new('sha512')
    b = w.getvalue()
    h.update(b)
    z = h.digest()
    r.append((z, b,))

    k = z
    parent = None
    LedgerEntry.credit_alice = random.randint(100, 1000)
    LedgerEntry.credit_bob = random.randint(100, 1000)

    generator = LedgerGenerator()
    for i in range(entry_count):
        try:
            v = generate_entry(data_dir, signer, generator, k, bob_name, parent)
        except OverflowError:
            break
        # \todo generate  key value already here
        parent = v[0]
        r.append(v)

    return r


if __name__ == '__main__':
    d = os.path.dirname(__file__)
    data_dir = os.path.join(d, 'testdata_resource')
    try:
        shutil.rmtree(data_dir)
    except FileNotFoundError:
        pass
    os.makedirs(data_dir)

    d = os.path.dirname(__file__)
    crypto_dir = os.path.join(d, 'testdata_crypt')
    try:
        shutil.rmtree(crypto_dir)
    except FileNotFoundError:
        pass
    os.makedirs(crypto_dir)

    d = db_init(d)

    env = lmdb.open(d)
    dbi = env.open_db()

    signer = LedgerSigner(crypto_dir)
    alice = signer.create_key('alice', outdir=data_dir)
    #bob = bob(d)

    keys = ['alice']

    alice_key = os.path.join(crypto_dir, 'alice.key.bin')
    try:
        os.unlink('kee.key')
    except FileNotFoundError:
        pass
    os.symlink(alice_key, 'kee.key')

    count_ledgers = os.environ.get('COUNT', '1')

    with env.begin(write=True) as tx:
        for i in range(int(count_ledgers)):
            bob_name = 'Bob ' + fake.last_name()
            keys.append(bob_name)
            bob = signer.create_key(bob_name, outdir=data_dir)
            
            c = random.randint(2, 20)
            r = generate_ledger(dbi, data_dir, signer, bob_name, entry_count=c, alice=alice, bob=bob)

            v = r.pop(0)

            z = v[0]
            k = LedgerHead.to_key(v[0])
            tx.put(k, v[1])

            for v in r:
                k = LedgerEntry.to_key(v[1], z)
                tx.put(k, v[1])
        
        for k in keys:
            pubk = signer.get_pubkey(k)
            name = signer.get_name(k).encode('utf-8')
            tx.put(PFX_LEDGER_PUBKEY + pubk, b'CN=' + name)

