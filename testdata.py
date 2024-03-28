import tempfile
import logging
import os
import shutil
import hashlib
import sys
import io
import random
import time

import lmdb
import varint

SIGNS_ALICE_CREDIT_DELTA_NEGATIVE = 1 << 0
SIGNS_BOB_CREDIT_DELTA_NEGATIVE = 1 << 1
SIGNS_ALICE_COLLATERAL_DELTA_NEGATIVE = 1 << 2
SIGNS_BOB_COLLATERAL_DELTA_NEGATIVE = 1 << 3

FLAGS_SIGNER_IS_BOB = 1 << 0

NOBODY = b'\x00' * 64
NOSIG = b'\x00' * 65
PFX_LEDGER_HEAD = b'\x01'
PFX_LEDGER_ENTRY = b'\x02'

logging.basicConfig(level=logging.DEBUG)
logg = logging.getLogger()


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
        self.body = body


    def __serialize_add(self, b, w):
        c = varint.encode(len(b))
        w.write(c)
        w.write(b)


    def serialize(self, w=sys.stdout.buffer):
        b = self.uoa.encode('utf-8')
        self.__serialize_add(b, w)

        b = varint.encode(self.uoa_decimals)
        self.__serialize_add(b, w)

        b = self.alice_pubkey_ref
        self.__serialize_add(b, w)

        b = self.bob_pubkey_ref
        self.__serialize_add(b, w)

        b = self.body
        self.__serialize_add(b, w)


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
        self.head = head
        self.flags = 0
        self.signs = 0
        self.parent = parent
        if self.parent == None:
            self.parent = b'\x00' * 64
        self.timestamp = time.time_ns()

        self.body = body

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


    def serialize(self, w=sys.stdout.buffer):
        b = self.flags.to_bytes(1)
        self.__serialize_add(b, w)
        
        b = self.parent
        self.__serialize_add(b, w)

        b = self.timestamp.to_bytes(8, byteorder='big')
        self.__serialize_add(b, w)

        b = self.signs.to_bytes(1)
        self.__serialize_add(b, w)

        realvalue = self.credit_delta
        if self.flags & FLAGS_SIGNER_IS_BOB:
            if (self.signs & SIGNS_BOB_CREDIT_DELTA_NEGATIVE):
                realvalue *= -1
        else:
            if (self.signs & SIGNS_ALICE_CREDIT_DELTA_NEGATIVE):
                realvalue *= -1

        logg.debug('encoding credit delta {}'.format(realvalue))
        b = varint.encode(self.credit_delta)
        if self.flags:
            self.__serialize_add(b'\x00', w)
        self.__serialize_add(b, w)
        if not self.flags:
            self.__serialize_add(b'\x00', w)

        logg.debug('encoding collateral delta {}'.format(self.collateral_delta))
        b = varint.encode(self.collateral_delta)
        self.__serialize_add(b, w)

        if self.flags:
            self.__serialize_add(b'\x00', w)
        self.__serialize_add(self.body, w)
        if not self.flags:
            self.__serialize_add(b'\x00', w)

        #if self.signer != None:
        #    self.signature = self.signer(b)

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
        ts = v[65:65+8]
        #logg.debug('ts {}: of {}'.format(ts.hex(), v.hex()))
        r += ts
        return r
  

def generate_entry(head, parent):
    o = LedgerEntry(head, parent=parent)
    w = io.BytesIO()
    r = o.serialize(w=w)
    h = hashlib.new('sha512')
    b = w.getvalue()
    h.update(b)
    z = h.digest()
    return (z, b,)


def generate_ledger(entry_count=3):
    r = []
    o = LedgerHead()
    w = io.BytesIO()
    o.serialize(w=w)
    h = hashlib.new('sha512')
    b = w.getvalue()
    h.update(b)
    z = h.digest()
    r.append((z, b,))

    k = z
    parent = None
    for i in range(entry_count):
        v = generate_entry(k, parent=parent)
        # \todo generate  key value already here
        parent = v[0]
        r.append(v)

    return r


if __name__ == '__main__':
    d = os.path.dirname(__file__)
    d = db_init(d)

    env = lmdb.open(d)
    dbi = env.open_db()

    count_ledgers = os.environ.get('COUNT', '1')

    with env.begin(write=True) as tx:
        for i in range(int(count_ledgers)):
            c = random.randint(1, 20)
            r = generate_ledger(entry_count=c)

            v = r.pop(0)

            z = v[0]
            k = LedgerHead.to_key(v[0])
            tx.put(k, v[1])

            for v in r:
                k = LedgerEntry.to_key(v[1], z)
                tx.put(k, v[1])


#pfx = b'\x00\x00\x00'
#pfx_two = b'\x00\x00\x01'

#keys = []
#vals = [
#    b"\x03foo\x03bar",
#    b"\x05xyzzy\x05plugh",
#    b"\x04inky\x05pinky",
#    b"\x06blinky\x05clyde",
#        ]
#
#for i in range(len(vals)):
#    k = b'\x01' + i.to_bytes(16, 'big')
#    #k += os.urandom(32)
#    h = hashlib.sha256()
#    h.update(vals[i])
#    k += h.digest()
#    keys.append(k)
#
#with env.begin(write=True) as tx:
#    for i in range(len(vals)):
#        v = i.to_bytes(2, byteorder='big')
#        tx.put(keys[i], vals[i])
#        #tx.put(keys[i], vals[i].encode('utf-8'))
#        #tx.put(pfx + v, vals[i].encode('utf-8'))
#    #tx.put(pfx_two + b'\x00\x00', b'xyzzy')
#    #tx.put(pfx_two + b'\x00\x01', b'plugh')
#
#with env.begin() as tx:
#    c = tx.cursor()
#    #if not c.set_range(b'\x00\x00\x01'):
#    #    raise ValueError("no key")
#    for k, v in c:
#        logg.debug('have {} {}'.format(k, v))
