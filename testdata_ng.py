import os
import sys
import io
import logging
import hashlib
from Crypto.Cipher import ChaCha20_Poly1305
from Crypto.PublicKey import ECC
import Crypto.IO.PKCS8
import Crypto.Util.asn1
import lmdb
import time
import shutil
import email.message
import random
from faker import Faker
from faker.providers import lorem
import varint

logging.basicConfig(level=logging.DEBUG)
logg = logging.getLogger()

fake = Faker()
fake.add_provider(lorem)

SIGNS_ALICE_CREDIT_DELTA_NEGATIVE = 1 << 0
SIGNS_BOB_CREDIT_DELTA_NEGATIVE = 1 << 1
SIGNS_ALICE_COLLATERAL_DELTA_NEGATIVE = 1 << 2
SIGNS_BOB_COLLATERAL_DELTA_NEGATIVE = 1 << 3

FLAGS_SIGNER_IS_BOB = 1 << 0

NOBODY = b'\x00' * 64
NOSIG = b'\x00' * 65
PFX_LEDGER_HEAD = b'\x01'
PFX_LEDGER_ENTRY = b'\x02'
PFX_LEDGER_COUNTERKEY = b'\x03'


def padbytes(b, padsize=4096):
    l = padsize - (len(b) % padsize)
    b += os.urandom(l)
    return b

h = hashlib.new('sha256')
h.update(b'1234')
z = h.digest()

def alice(w):
    k = ECC.generate(curve='Ed25519')
    pk_pkcs8 = k.export_key(format='DER')
    pk_der = Crypto.IO.PKCS8.unwrap(pk_pkcs8)
    pk = Crypto.Util.asn1.DerOctetString().decode(pk_der[1], strict=True).payload
    pubk = k.public_key().export_key(format='raw')

    wt = io.BytesIO()
    wt.write(b"(8:key-data(10:public-key(3:ecc(5:curve7:Ed25519)(1:q32:")
    wt.write(pubk)
    wt.write(b")))(11:private-key(3:ecc(5:curve7:Ed25519)(1:q32:")
    wt.write(pubk)
    wt.write(b")(1:d32:")
    wt.write(pk)
    wt.write(b"))))")
    b = wt.getvalue()
    l = len(b)
    bl = l.to_bytes(4, byteorder='little')

    nonce = os.urandom(12)
    cph = ChaCha20_Poly1305.new(key=z, nonce=nonce)
    r = cph.encrypt(bl + b)
    r = padbytes(r)
    w.write(nonce + r)

    return pubk


def bob(d):
    k = ECC.generate(curve='Ed25519')
    pubk = k.public_key().export_key(format='raw')


    wt = io.BytesIO()
    wt.write(b"(8:key-data(10:public-key(3:ecc(5:curve7:Ed25519)(1:q32:")
    wt.write(pubk)
    wt.write(b"))))")
    b = wt.getvalue()

    env = lmdb.open(d)
    dbi = env.open_db()

    k = PFX_LEDGER_COUNTERKEY = b'\x03'

    with env.begin(write=True) as tx:
            tx.put(k + pubk, b'bob')

    return pubk


def db_init(d):
    d = os.path.join(d, 'testdata_mdb')
    logg.info('using d for db' + d)

    try:
        shutil.rmtree(d)
    except FileNotFoundError:
        pass
    os.makedirs(d)
    return d


def to_absflag(v):
    flag = False
    if v < 0:
        flag = True
    return (abs(v), flag,)


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


class LedgerHead:

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


    def __serialize_add(self, b, w):
        c = varint.encode(len(b))
        w.write(c)
        w.write(b)


    def __data_add(self, data_dir, k, v):
        fp = os.path.join(data_dir, k.hex())
        f = open(fp, 'wb')
        #logg.info("fp {}".format(fp))
        f.write(v)
        f.close()


    def serialize(self, data_dir, w=sys.stdout.buffer):
        b = self.uoa.encode('utf-8')
        self.__serialize_add(b, w)

        b = varint.encode(self.uoa_decimals)
        self.__serialize_add(b, w)

        b = self.alice_pubkey_ref
        self.__serialize_add(b, w)

        b = self.bob_pubkey_ref
        self.__serialize_add(b, w)

        (k, b) = self.body.kv()
        self.__data_add(data_dir, k, b)
        self.__serialize_add(k, w)


    @staticmethod
    def to_key(b):
        r = b''
        r += PFX_LEDGER_HEAD
        v = time.time_ns()
        b = v.to_bytes(8, byteorder='big')
        r += b

        return r


class LedgerEntry:

    credit_delta_min = -1000
    credit_delta_max = 1000
    collateral_delta_min = 0
    collateral_delta_max = 0

    def __init__(self, head, parent=None, body=NOBODY, signer=None):
        random.seed(int(time.time_ns()))
        self.head = head
        self.flags = 0
        self.signs = 0
        self.parent = parent
        if self.parent == None:
            self.parent = b'\x00' * 64
        self.timestamp = time.time_ns()

        self.body = LedgerEntryContent()

        v = random.randint(self.credit_delta_min, self.credit_delta_max)
        self.flags = v % 2
        (v, neg) = to_absflag(v)
        self.credit_delta = v
        if neg:
            if self.flags:
                self.signs |= SIGNS_BOB_CREDIT_DELTA_NEGATIVE
            else:
                self.signs |= SIGNS_ALICE_CREDIT_DELTA_NEGATIVE

        v = random.randint(self.collateral_delta_min, self.collateral_delta_max)
        self.response_value = v % 2
        (v, neg) = to_absflag(v)
        self.collateral_delta = v
        if neg:
            if self.flags:
                self.signs |= SIGNS_BOB_COLLATERAL_DELTA_NEGATIVE
            else:
                self.signs |= SIGNS_ALICE_COLLATERAL_DELTA_NEGATIVE

        #self.request_signature = NOSIG
        #self.response_signature = NOSIG
        self.request_signature = os.urandom(65)
        self.response_signature = os.urandom(65)
        self.signer = signer


    def __serialize_add(self, b, w):
        c = varint.encode(len(b))
        w.write(c)
        w.write(b)


    def __data_add(self, data_dir, k, v):
        fp = os.path.join(data_dir, k.hex())
        f = open(fp, 'wb')
        #logg.info("fp {}".format(fp))
        f.write(v)
        f.close()


    def serialize(self, data_dir, w=sys.stdout.buffer):
        b = self.flags.to_bytes(1)
        self.__serialize_add(b, w)
        
        b = self.parent
        self.__serialize_add(b, w)

        b = self.timestamp.to_bytes(8, byteorder='big')
        self.__serialize_add(b, w)

        b = self.signs.to_bytes(1)
        self.__serialize_add(b, w)

#        realvalue = self.credit_delta
#        if self.flags & FLAGS_SIGNER_IS_BOB:
#            if (self.signs & SIGNS_BOB_CREDIT_DELTA_NEGATIVE):
#                realvalue *= -1
#        else:
#            if (self.signs & SIGNS_ALICE_CREDIT_DELTA_NEGATIVE):
#                realvalue *= -1

    
        b = varint.encode(self.credit_delta)
        if self.flags:
            self.__serialize_add(varint.encode(0), w)
        self.__serialize_add(b, w)
        if not self.flags:
            self.__serialize_add(varint.encode(0), w)
        

        #if self.flags:
        #    self.__serialize_add(b'\x00', w)
        #logg.debug('encode flags {} credit {} collateral {}'.format(self.flags, self.credit_delta, self.collateral_delta))
        b = varint.encode(self.collateral_delta)
        if not self.flags:
            self.__serialize_add(varint.encode(0), w)
        self.__serialize_add(b, w)
        if not self.flags:
            self.__serialize_add(varint.encode(0), w)

        #if self.signer != None:
        #    self.signature = self.signer(b)

        (k, b) = self.body.kv()
        self.__data_add(data_dir, k, b)
        self.__serialize_add(k, w)

        self.__serialize_add(self.request_signature, w)

        b = self.response_value.to_bytes(1)
        self.__serialize_add(b, w)
        
        self.__serialize_add(self.response_signature, w)

        return b


    @staticmethod
    def to_key(v, k):
        r = b''
        r += PFX_LEDGER_ENTRY
        r += k
        ts = v[68:68+8]
        #logg.debug('ts {}: of {}'.format(ts.hex(), v.hex()))
        r += ts
        return r
  

def generate_entry(data_dir, head, parent):
    o = LedgerEntry(head, parent=parent)
    w = io.BytesIO()
    r = o.serialize(data_dir, w=w)
    h = hashlib.new('sha512')
    b = w.getvalue()
    h.update(b)
    z = h.digest()
    return (z, b,)


def generate_ledger(data_dir, entry_count=3, alice=None, bob=None):
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
    for i in range(entry_count):
        v = generate_entry(data_dir, k, parent=parent)
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

    d = db_init(d)

    f = open('key.bin', 'wb')
    alice = alice(f)
    f.close()
    bob = bob(d)

    print("bo {}\nalice {}\n".format(alice.hex(), bob.hex()))

    env = lmdb.open(d)
    dbi = env.open_db()

    count_ledgers = os.environ.get('COUNT', '1')

    with env.begin(write=True) as tx:
        for i in range(int(count_ledgers)):
            c = random.randint(1, 20)
            r = generate_ledger(data_dir, entry_count=c, alice=alice, bob=bob)

            v = r.pop(0)

            z = v[0]
            k = LedgerHead.to_key(v[0])
            tx.put(k, v[1])

            for v in r:
                k = LedgerEntry.to_key(v[1], z)
                tx.put(k, v[1])
