import enum
import os
import sys
import io
import zlib
import base64
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
from pyasn1.codec.native.decoder import decode
from pyasn1.type.univ import Any
from pygcrypt.gctypes.sexpression import SExpression
from pygcrypt.gctypes.key import Key as GKey

from testdata_asn1schema import KeeEntryHead
from testdata_asn1schema import KeeEntry
from testdata_asn1schema import KeeTransport

logging.basicConfig(level=logging.DEBUG)
logg = logging.getLogger()

fake = Faker()
fake.add_provider(lorem)


FLAGS_SIGNER_IS_BOB = 1 << 0

def to_key_filename(keyname):
    filename = keyname.lower()
    filename = filename.replace(" ", "_")
    return filename


class LedgerMode(enum.IntEnum):
    REQUEST = 0
    RESPONSE = 1
    FINAL = 2


class LedgerRole(enum.Enum):
    ALICE = 'alice'
    BOB = 'bob'


class TransportCmd(enum.Enum):
    IMPORT = b'\x00'
    ID = b'\x01'
    LEDGER = b'\x02'


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
    d = os.path.join(d, 'testdata', 'mdb')
    logg.info('using d for db' + d)

    try:
        shutil.rmtree(d)
    except FileNotFoundError:
        pass
    os.makedirs(d)
    return d 


def data_add(data_dir, k, v):
    if data_dir == None:
        return
    fp = os.path.join(data_dir, k.hex())
    f = open(fp, 'wb')
    f.write(v)
    f.close()


class LedgerContent(email.message.EmailMessage):

    def __init__(self, subject=None, body=None):
        super(LedgerContent, self).__init__()
        self.set_default_type("text/plain")
        if subject == None:
            subject = fake.sentence()
        self.add_header("Subject", subject)
        if body == None:
            body = fake.paragraph()
        self.set_content(body)


    def kv(self):
        b = self.as_bytes()
        h = hashlib.new("sha512")
        h.update(b)
        z = h.digest()
        return (z, b,)


class LedgerHeadContent(LedgerContent):
    pass


class LedgerItemContent(LedgerContent):
    pass


# TODO: do everything with pygcrypt, or calc keygrip with pycryptodome 8|
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



    def __write_key(self, keyname, outdir, pin, alias=None):
        (pk, pubk) = self.keypair[keyname]
        wt = io.BytesIO()
        wt.write(b"(8:key-data(10:public-key(3:ecc(5:curve7:Ed25519)(5:flags5:eddsa)(1:q32:")
        wt.write(pubk)
        wt.write(b")))(11:private-key(3:ecc(5:curve7:Ed25519)(5:flags5:eddsa)(1:q32:")
        wt.write(pubk)
        wt.write(b")(1:d32:")
        wt.write(pk)
        wt.write(b"))))")
        b = wt.getvalue()

        filename = to_key_filename(keyname)
        fp = os.path.join(self.crypto_dir, filename + '.key.sexp')
        w = open(fp, 'wb')
        w.write(b)
        w.close()

        sexp = SExpression(b)
        gk = GKey(sexp)

        l = len(b)
        bl = l.to_bytes(4, byteorder='little')
        h = hashlib.new('sha256')
        h.update(pin.encode('utf-8'))
        z_pin = h.digest()
        nonce = os.urandom(12)
        cph = ChaCha20_Poly1305.new(key=z_pin, nonce=nonce)
        r = cph.encrypt(bl + b)
        r = padbytes(r)

        fp = os.path.join(self.crypto_dir, filename + '.key.bin')
        w = open(fp, 'wb')
        w.write(nonce + r)
        w.close()

        # symlink key to keygrip
        lp = os.path.join(self.crypto_dir, gk.keygrip)
        os.symlink(fp, lp)

        # symlink key to alias
        if alias != None:
            lp = os.path.join(self.crypto_dir, alias + '.key.bin')
            os.symlink(fp, lp)

        wt = io.BytesIO()
        wt.write(b"(8:key-data(10:public-key(3:ecc(5:curve7:Ed25519)(5:flags5:eddsa)(1:q32:")
        wt.write(pubk)
        wt.write(b"))))")
        b = wt.getvalue()
        fp = os.path.join(self.crypto_dir, filename + '.pubkey.sexp')
        w = open(fp, "wb")
        w.write(b)
        w.close()

        fp = os.path.join(self.crypto_dir, filename + '.pubkey.txt')
        w = open(fp, "w")
        w.write(pubk.hex())
        w.close()

        return gk.keygrip


    def create_key(self, keyname, outdir=None, pin='1234', alias=None):
        k = ECC.generate(curve='Ed25519')
        pk_pkcs8 = k.export_key(format='DER')
        pk_der = Crypto.IO.PKCS8.unwrap(pk_pkcs8)
        pk = Crypto.Util.asn1.DerOctetString().decode(pk_der[1], strict=True).payload
        pubk = k.public_key().export_key(format='raw')

        self.signer[keyname] = eddsa.new(k, 'rfc8032')
        self.keypair[keyname] = (pk, pubk)
        self.pubkey_rindex[pubk] = keyname

        keygrip = self.__write_key(keyname, outdir, pin, alias=alias)
        
        self.names[keyname] = fake.name()

        return (pubk, keygrip,)


    def sign(self, keyname, msg):
        h = hashlib.sha512()
        h.update(msg)
        z = h.digest()

        fp = os.path.join(self.crypto_dir, z.hex())
        w = open(fp, 'wb')
        w.write(msg)
        w.close()
    
        b = self.signer[keyname].sign(z)

        logg.debug('signature material\t{}\n\t{}'.format(msg[:64].hex(), msg[64:].hex()))
        logg.debug('signature for {}: {}'.format(z.hex(), b.hex()))

        return b



class LedgerBundle:

    def __init__(self, data_dir, ledger):
        self.data_dir = data_dir
        self.o = KeeTransport()
        self.o.setComponentByPosition(0, ledger.to_asn1(data_dir))


    def to_asn1(self, ledger_item, mode):
        self.o.setComponentByPosition(1, ledger_item.to_asn1(data_dir, mode))
        return self.o


    def serialize(self, ledger_item, mode, w=sys.stdout.buffer):
        o = self.to_asn1(ledger_item, mode)
        b = der_encode(o)
        b = TransportCmd.LEDGER.value + b
        if w == None:
            return b
        w.write(b)


    def encode(self, ledger_item, mode, w=sys.stdout.buffer):
        b = self.serialize(ledger_item, mode, w=None)
        b = zlib.compress(b, level=9)
        b = base64.b64encode(b)
        w.write(b) 


class Ledger:
    pass

#    @classmethod
#    def data_add(self, data_dir, k, v):
#        fp = os.path.join(data_dir, k.hex())
#        f = open(fp, 'wb')
#        f.write(v)
#        f.close()


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


    def delta(self, collateral=False, credit=True, signer_role=None):
        delta_credit = 0
        delta_collateral = 0
  
        single_is_bob = False

        initial_credit_role = None
        if self.count < 2:
            if signer_role == None:
                if self.count == 0:
                    initial_credit_role = LedgerRole.ALICE
                else:
                    initial_credit_role = LedgerRole.BOB
            else: 
                initial_credit_role = signer_role

        if initial_credit_role != None:
            if initial_credit_role == LedgerRole.ALICE:
                delta_credit = self.credit_alice
                delta_collateral = self.collateral_alice
            else:
                delta_credit = self.credit_bob
                delta_collateral = self.collateral_bob
                single_is_bob = True
        else:
            if self.credit_bob == 0 and self.credit_alice == 0:
                raise OverflowError("Both alice and bob are broke :'(")

        if delta_credit == 0:
            if signer_role == None:
                single_is_bob = bool(random.randint(0, 1))
            elif signer_role == LedgerRole.ALICE:
                single_is_bob = True

            if self.credit_bob == 0:
                if signer_role == LedgerRole.BOB:
                    raise OverflowError("bob is broke")
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
        self.body = LedgerHeadContent()
        (k, v) = self.body.kv()

        logg.info('new ledger header with alice {} bob {} body {}'.format(self.alice_pubkey_ref.hex(), self.bob_pubkey_ref.hex(), k.hex()))


    def to_asn1(self, data_dir):
        o = KeeEntryHead()
        o['uoa'] = self.uoa
        o['uoaDecimals'] = self.uoa_decimals
        o['alicePubKey'] = self.alice_pubkey_ref
        o['bobPubKey'] = self.bob_pubkey_ref
        (k, v) = self.body.kv()
        #self.data_add(data_dir, k, v)
        data_add(data_dir, k, v)
        o['body'] = k
        return o


    def serialize(self, data_dir, w=sys.stdout.buffer):
        o = self.to_asn1(data_dir)
        b = der_encode(o)
        logg.debug('ledger header serialize ({}): {}'.format(len(b), b.hex()))
        w.write(b)


    @staticmethod
    def to_key(b):
        r = b''
        r += PFX_LEDGER_HEAD
        t = time.time_ns()
        v = int(t / 1000000000)
        b = v.to_bytes(4, byteorder='big')
        v = t - (v * 1000000000)
        b += v.to_bytes(4, byteorder='big')
        r += b

        return r


class LedgerItem(Ledger):

    credit_alice = 0
    credit_bob = 0
    collateral_alice = 0
    collateral_bob = 0
    ms = 0

    def __init__(self, head, signer, generator, signer_name=None, parent=None, body=NOBODY, bob_name='bob'):
        self.head = head
        self.parent = parent
        if self.parent == None:
            self.parent = b'\x00' * 64
        #self.timestamp = time.time_ns()
        self.timestamp = b''
        t = time.time_ns()
        v = int(t / 1000000000)
        self.timestamp += v.to_bytes(4, byteorder='big')
        v = t - (v * 1000000000)
        self.timestamp += v.to_bytes(4, byteorder='big')

        self.body = LedgerItemContent()

        signer_role = None
        if signer_name != None:
            signer_role=LedgerRole(signer_name)
        delta = generator.delta(signer_role=signer_role)
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


    def to_asn1(self, data_dir, mode=LedgerMode.FINAL):
        o = KeeEntry()
        o['parent'] = self.parent
        o['timestamp'] = self.timestamp
        o['creditDelta'] = self.credit_delta
        o['collateralDelta'] = self.collateral_delta

        (k, v) = self.body.kv()
        #self.data_add(data_dir, k, v)
        data_add(data_dir, k, v)
        o['body'] = k

        o['response'] = False
        o['signatureRequest'] = NOSIG
        o['signatureResponse'] = NOSIG

        if mode == LedgerMode.REQUEST:
            return o

        logg.debug('encoding new ledger_item for request signature {}: {}'.format(self.head.hex(), o))
        b = der_encode(o)
        self.request_signature = self.signer.sign(self.signer_sequence[0], self.head + b)
        o['signatureRequest'] = self.request_signature

        if mode == LedgerMode.RESPONSE:
            return o 

        if mode != LedgerMode.FINAL:
            raise ValueError("invalid ledger mode: {}".format(mode))

        o['response'] = True
        b = der_encode(o)
        self.response_signature = self.signer.sign(self.signer_sequence[1], self.head + b)
        o['signatureResponse'] = self.response_signature
        return o


    def serialize(self, data_dir, mode=LedgerMode.FINAL, w=sys.stdout.buffer):
        o = self.to_asn1(data_dir, mode=mode)
        b = der_encode(o)
        flag = b'\x00'
        if self.signer_sequence[0] != 'alice':
            flag = b'\x01'
        w.write(b + flag)

        LedgerItem.ms += 1
        if LedgerItem.ms > 999:
            LedgerItem.ms = 0


    @staticmethod
    def to_key(v, k):
        r = b''
        r += PFX_LEDGER_ENTRY
        r += k
       
        o = der_decode(v, asn1Spec=KeeEntry())
        #ts = o[0]['timestamp']
        #tsb = int(ts).to_bytes(8, byteorder='big')
        tsb = o[0]['timestamp'].asOctets()
        #logg.debug('ts {} ({}): of {}'.format(ts, tsb, v.hex()))
        r += tsb
        return r
  

def generate_ledger_item(data_dir, signer, generator, head, bob_name, parent):
    o = LedgerItem(head, signer, generator, parent=parent, bob_name=bob_name)
    w = io.BytesIO()
    r = o.serialize(data_dir, w=w)
    h = hashlib.new('sha512')
    b = w.getvalue()
    h.update(b)
    z = h.digest()
    return (z, b, o,)


def generate_ledger(dbi, data_dir, signer, bob_name, ledger_item_count=3, alice=None, bob=None):
    r = []
    o = LedgerHead(alice_key=alice, bob_key=bob)
    w = io.BytesIO()
    o.serialize(data_dir, w=w)
    h = hashlib.new('sha512')
    b = w.getvalue()
    h.update(b)
    z = h.digest()
    r.append((z, b, o,))

    k = z
    parent = None
    LedgerItem.credit_alice = random.randint(100, 1000)
    LedgerItem.credit_bob = random.randint(100, 1000)

    generator = LedgerGenerator()
    for i in range(ledger_item_count):
        try:
            v = generate_ledger_item(data_dir, signer, generator, k, bob_name, parent)
        except OverflowError:
            break
        # \todo generate  key value already here
        parent = v[0]
        r.append(v)

    return r


if __name__ == '__main__':
    d = os.path.dirname(__file__)
    data_dir = os.path.join(d, 'testdata', 'resource')
    try:
        shutil.rmtree(data_dir)
    except FileNotFoundError:
        pass
    os.makedirs(data_dir)
    
    v = b'foo'
    h = hashlib.sha512()
    h.update(v)
    k = h.digest()
    data_add(data_dir, k, v)

    o = LedgerContent(subject='foo', body='bar')
    (k, v) = o.kv()
    data_add(data_dir, k, v)

    d = os.path.dirname(__file__)
    crypto_dir = os.path.join(d, 'testdata', 'crypt')
    try:
        shutil.rmtree(crypto_dir)
    except FileNotFoundError:
        pass
    os.makedirs(crypto_dir)

    d = os.path.dirname(__file__)
    crypto_dir_r = os.path.join(d, 'testdata', 'crypt_reverse')
    try:
        shutil.rmtree(crypto_dir_r)
    except FileNotFoundError:
        pass
    os.makedirs(crypto_dir_r)

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
    alice_key_sym = 'kee.key'
    os.symlink(alice_key, alice_key_sym)
    alice_key_sym = os.path.join(crypto_dir, alice_key_sym)
    os.symlink(alice_key, alice_key_sym)

    count_ledgers = os.environ.get('COUNT', '1')
    items_min_in = os.environ.get('ITEM_MIN', '1')
    items_max_in = os.environ.get('ITEM_MAX', '20')

    mainbob_keygrip = None
    with env.begin(write=True) as tx:
        for i in range(int(count_ledgers)):
            bob_name = 'Bob ' + fake.last_name()
            keys.append(bob_name)
            alias = None
            if i == 0:
                alias = 'bob'
            bob = signer.create_key(bob_name, outdir=data_dir, pin='4321', alias=alias)
            if i == 0:
                mainbob_keygrip = bob[1]
#            bob_key = os.path.join(crypto_dir, 'bob.key.bin')
#            bob_key_sym = os.path.join(crypto_dir_r, 'kee.key')
#            try:
#                os.unlink('kee.key')
#            except FileNotFoundError:
#                pass
#            os.symlink(bob_key, bob_key_sym)
            
            c = 2 + random.randint(int(items_min_in), int(items_max_in))

            r = generate_ledger(dbi, data_dir, signer, bob_name, ledger_item_count=c, alice=alice[0], bob=bob[0])

            v = r.pop(0)

            ledger_object = v[2]

            z = v[0]
            k = LedgerHead.to_key(v[0])
            tx.put(k, v[1])
            # reverse lookup
            kr = b'\xff' + v[0]
            tx.put(kr, k[1:])

            for v in r:
                k = LedgerItem.to_key(v[1], z)
                tx.put(k, v[1])

        for k in keys:
            pubk = signer.get_pubkey(k)
            name = signer.get_name(k).encode('utf-8')
            tx.put(PFX_LEDGER_PUBKEY + pubk, b'CN=' + name)

    # generate ledger import
    bob_name = 'Bob ' + fake.last_name()
    keys.append(bob_name)
    bob = signer.create_key(bob_name, outdir=data_dir)
    bob_key = os.path.join(crypto_dir, 'bob.key.bin')
    bob_key_sym = os.path.join(crypto_dir_r, 'kee.key')
    try:
        os.unlink('kee.key')
    except FileNotFoundError:
        pass
    os.symlink(bob_key, bob_key_sym)
    bob_keygrip_sym = os.path.join(crypto_dir_r, mainbob_keygrip)
    os.symlink(bob_key, bob_keygrip_sym)


    r = generate_ledger(dbi, data_dir, signer, bob_name, ledger_item_count=1, alice=alice[0], bob=bob[0])
    d = os.path.dirname(__file__)
    import_dir = os.path.join(d, 'testdata', 'import')
    try:
        shutil.rmtree(import_dir)
    except FileNotFoundError:
        pass
    os.makedirs(import_dir)

    ledger_object = r[0][2]
    ledger_item_object = r[1][2]
    importer = LedgerBundle(data_dir, ledger_object)
    w = io.BytesIO()
    fp = os.path.join(import_dir, 'request')
    f = open(fp, 'wb')
    importer.encode(ledger_item_object, LedgerMode.REQUEST, w=f)
    f.close()

    w = io.BytesIO()
    fp = os.path.join(import_dir, 'response')
    f = open(fp, 'wb')
    importer.encode(ledger_item_object, LedgerMode.RESPONSE, w=f)
    f.close()

    w = io.BytesIO()
    fp = os.path.join(import_dir, 'final')
    f = open(fp, 'wb')
    importer.encode(ledger_item_object, LedgerMode.FINAL, w=f)
    f.close()
