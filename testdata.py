import lmdb
import tempfile
import logging
import os
import shutil
import hashlib

logging.basicConfig(level=logging.DEBUG)
logg = logging.getLogger()

d = os.path.dirname(__file__)
d = os.path.join(d, 'testdata_mdb')
logg.info('using ' + d)

try:
    shutil.rmtree(d)
except FileNotFoundError:
    pass
os.makedirs(d)
env = lmdb.open(d)
dbi = env.open_db()

#pfx = b'\x00\x00\x00'
#pfx_two = b'\x00\x00\x01'

keys = []
vals = [
    b"\x03foo\x03bar",
    b"\x05xyzzy\x05plugh",
    b"\x04inky\x05pinky",
    b"\x06blinky\x05clyde",
        ]

for i in range(len(vals)):
    k = b'\x01' + i.to_bytes(16, 'big')
    #k += os.urandom(32)
    h = hashlib.sha256()
    h.update(vals[i])
    k += h.digest()
    keys.append(k)

with env.begin(write=True) as tx:
    for i in range(len(vals)):
        v = i.to_bytes(2, byteorder='big')
        tx.put(keys[i], vals[i])
        #tx.put(keys[i], vals[i].encode('utf-8'))
        #tx.put(pfx + v, vals[i].encode('utf-8'))
    #tx.put(pfx_two + b'\x00\x00', b'xyzzy')
    #tx.put(pfx_two + b'\x00\x01', b'plugh')

with env.begin() as tx:
    c = tx.cursor()
    #if not c.set_range(b'\x00\x00\x01'):
    #    raise ValueError("no key")
    for k, v in c:
        logg.debug('have {} {}'.format(k, v))
