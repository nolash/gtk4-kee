import os
import sys
import io
import logging
import hashlib
from Crypto.Cipher import ChaCha20_Poly1305
from Crypto.PublicKey import ECC
import Crypto.IO.PKCS8
import Crypto.Util.asn1


def padbytes(b, padsize=4096):
    l = padsize - (len(b) % padsize)
    b += os.urandom(l)
    return b

h = hashlib.new('sha256')
h.update(b'1234')
z = h.digest()

k = ECC.generate(curve='Ed25519')
pk_pkcs8 = k.export_key(format='DER')
pk_der = Crypto.IO.PKCS8.unwrap(pk_pkcs8)
pk = Crypto.Util.asn1.DerOctetString().decode(pk_der[1], strict=True).payload
pubk = k.public_key().export_key(format='raw')

w = io.BytesIO()
w.write(b"(8:key-data(10:public-key(3:ecc(5:curve7:Ed25519)(1:q32:")
w.write(pubk)
w.write(b")))(11:private-key(3:ecc(5:curve7:Ed25519)(1:q32:")
w.write(pubk)
w.write(b")(1:d32:")
w.write(pk)
w.write(b"))))")
b = w.getvalue()
l = len(b)
bl = l.to_bytes(4, byteorder='little')

nonce = os.urandom(12)
cph = ChaCha20_Poly1305.new(key=z, nonce=nonce)
r = cph.encrypt(bl + b)
r = padbytes(r)
sys.stdout.buffer.write(nonce + r)

#tmpl = (8:key-data(10:public-key(3:ecc(5:curve7:Ed25519)(1:q32:\xae3\xe12\xec\x9e:\xa3-\xa3\x0b\x122}\xbc\xdb\xd8\xdc\x03\xea\x989D[S\xbaocs\xfb\x00\xce)))(11:private-key(3:ecc(5:curve7:Ed25519)(1:q32:\xae3\xe12\xec\x9e:\xa3-\xa3\x0b\x122}\xbc\xdb\xd8\xdc\x03\xea\x989D[S\xbaocs\xfb\x00\xce)(1:d32:k\x90\x88\xb5\x8cyn\xef]b\xd8\x80\x19\xd1\xf8\xda\xe2\xc0\x1b\xe9V\t\x07h7\x05\xb7\xd8\x85bu0))))


# b'(8:key-data(10:public-key(3:ecc(5:curve7:Ed25519)(1:q32:\xae3\xe12\xec\x9e:\xa3-\xa3\x0b\x122}\xbc\xdb\xd8\xdc\x03\xea\x989D[S\xbaocs\xfb\x00\xce)))(11:private-key(3:ecc(5:curve7:Ed25519)(1:q32:\xae3\xe12\xec\x9e:\xa3-\xa3\x0b\x122}\xbc\xdb\xd8\xdc\x03\xea\x989D[S\xbaocs\xfb\x00\xce)(1:d32:k\x90\x88\xb5\x8cyn\xef]b\xd8\x80\x19\xd1\xf8\xda\xe2\xc0\x1b\xe9V\t\x07h7\x05\xb7\xd8\x85bu0))))'

#f = open('key.bin', 'rb')
#l = int.from_bytes(f.read(4), byteorder='little')
#nonce = f.read(12)
#ctxt = f.read()
#f.close()
#
#cph = ChaCha20_Poly1305.new(key=z, nonce=nonce)
#txt = cph.decrypt(ctxt)
#print(txt[:l])
