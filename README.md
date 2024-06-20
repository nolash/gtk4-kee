# kee

**The upstream respository is [git://holbrook.no/kee-gtk4.git](git://holbrook.no/kee-gtk4.git)**

I am building a cryptographically counter-signed IOU application with linux phones ([GTK4](https://www.gtk.org/), [phosh](https://phosh.mobi/)) as the primary targets.

It will maintain bi-lateral mutual channels of credit and collateral. Each change is linked as a chain, and documentation for each change in embedded as [email-link MIME structures](https://datatracker.ietf.org/doc/html/rfc2049)


## Dependencies

Below are the library versions of the [archlinux](https://archlinux.org/) component packages I am using for dependencies and tooling while working on my own system.

| package | version |
|---|---|
| gcc | 13.2.1 |
| graphicsmagick | 1.3.42 |
| gst-plugins-bad  | 1.24.0 |
| gstreamer | 1.24.0 |
| gtk4 | 4.12.5 |
| libb64 | 1.2.1 |
| libgcrypt | 1.10.3 |
| qrencode | 4.1.1 |
| libxdg-basedir | 1.2.3 |
| lmdb | 0.9.32 |
| openldap | 2.6.7 |
| rustup | 1.26.0 |
| zbar | 0.23.90 |
| zlib | 1.3.1 |

The `gst-plugins-bad` provides [libgstzbar.so](https://gstreamer.freedesktop.org/documentation/zbar/index.html?gi-language=c#zbar-page), used as part of QR scanning.


### Other sources

The code expects objects from these source trees to be available somehow:

|project | upstream source | version | tar.gz sha256 |
|---|---|---|---|
| liblash* | [https://holbrook.no/src/liblash](https://holbrook.no/src/liblash) `(193dd4032e8088ada2dc9dcedab8d7486e9305f5)` | `0.1.0` | `333cdf49bb7e9f44b37e5bc9f594a01e4b3b8ecb3fcd3d9ffec3ea6dcdeaec7b` |
| libcmime | [https://github.com/spmfilter/libcmime.git](https://github.com/spmfilter/libcmime.git) `(dd21eb096d162656e30243f60fc4bc35ad39ae6e)` | `0.2.2` | `18a8d46ebec575a79212acc2dc6af7fd7bdeba3a9b85a70677ed0b7785c5c04e` |
| gst-plugins-rs | [https://gitlab.freedesktop.org/gstreamer/gst-plugins-rs](https://gitlab.freedesktop.org/gstreamer/gst-plugins-rs) `(a84bbc66f30573b62871db163c48afef75adf6ec)` | `1.22.10 (gstreamer)` | `691d5d52f59ec6322a2f6ddc1039ef47c9ac5e6328e2df1ef920629b46c659df` |

You can close `liblash` with `git clone git://holbrook.no/liblash`

(\*liblash is now bundled in the `src/aux` directory)


## Building

**There are instructions on building and running with [qemu](https://www.qemu.org/) and [Debian Bookworm (12)](https://cdimage.debian.org/debian-cd/current/amd64/iso-cd/debian-12.5.0-amd64-netinst.iso) in `README.qemu.adoc`**

There is no magic pathfinder implemented currently. Thus, you must configure the environment for building and linking the external sources yourself.

Here is what I use in my local setup.


### libcmime

I can't seem to set the cmake build prefix so that the `libcmime.pc` file is generated correctly. Unfortunately, `pkg-config` is thus off the table.

```
git clone -b 0.2.2 https://github.com/spmfilter/libcmime.git
https://github.com/spmfilter/libcmime.git
mkdir -p libcmime/build
cd libcmime/build
cmake ..
make install DESTDIR=.
```

Your include path, relative to the repository root directory, will be `<repo_root>/build/usr/include/lib64` and your ld path will be `build/usr/include/lib64`.


### libgstgtk4 (gst-plugin-rs)

The `gtk4` part of the code uses the `gtk4paintablesink` gstreamer plugin (for QR scanning). It is part of the `gst-plugins-rs` source above, built as the `libgstgtk4.so` shared library file.

The specific package to build is `gst-plugin-gtk4`, which builds the plugin at version `0.9.13`.

I was able to build with toolchain `1.75-x86_64-unknown-linux-gnu`.

```
rustup default 1.75.0
cargo install cargo-c
git clone -b gstreamer-1.22.10 https://gitlab.freedesktop.org/gstreamer/gst-plugins-rs
cd gst-plugins-rs
cargo cbuild -p gst-plugin-gtk4
```


## Environment 

```
#!/bin/bash

set -x

export GST_PLUGIN_PATH=/home/lash/src/build/gst-plugins-rs/target/x86_64-unknown-linux-gnu/debug/
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/home/lash/src/build/libcmime/0.2.2/build/usr/local/lib64/pkgconfig
export LIBRARY_PATH=$LIBRARY_PATH:/home/lash/src/build/libcmime/0.2.2/build/usr/local/lib64:/home/lash/src/home/liblash/src
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/lash/src/build/libcmime/0.2.2/build/usr/local/lib64:/home/lash/src/home/liblash/src
export C_INCLUDE_PATH=$C_INCLUDE_PATH:/home/lash/src/build/libcmime/0.2.2/build/usr/local/include:/home/lash/src/home/liblash/src

make run

set +x
```

Here is some background if you need:

* [C_INCLUDE_PATH](https://gcc.gnu.org/onlinedocs/cpp/Environment-Variables.html)
* [LIBRARY_PATH](https://gcc.gnu.org/onlinedocs/gcc/Environment-Variables.html#index-LIBRARY_005fPATH)
* [LD_LIBRARY_PATH](https://www.man7.org/linux/man-pages/man8/ld.so.8.html#ENVIRONMENT)
* [Difference between LD_LIBRARY_PATH and LIBRARY_PATH](https://www.baeldung.com/linux/library_path-vs-ld_library_path)
* [PKG_CONFIG_PATH](https://man.archlinux.org/man/pkgconf.1.en#ENVIRONMENT)
* [GST_PLUGIN_PATH](https://gstreamer.freedesktop.org/documentation/gstreamer/running.html?gi-language=c)
* [cargo-c](https://github.com/lu-zero/cargo-c)


## Running

In order to run the GTK application in its current condition, generated test data should be used as data path.

Take care to install python dependencies in `requirements.txt` before the first command. Using a [virtual environment](https://docs.python.org/3/library/venv.html) is recommended. The author is using `python 3.12.x`.

```
make testdata
export KEE_PATH=$(realpath ./testdata)
make run
``` 

The testdata contains private keys for both Alice and Bob in the generated entries. To start as Bob, add before running:

```
export KEE_KEY_PATH=$(realpath ./testdata/crypt_reverse)
```


## Protocol

TODO: The terms need to be tightened up; don't use different terms for same thing.

The serialization format of the credit issuance is defined in `src/asb1/schema_entry.txt`. The serialization method is _der-encoding_.

More about `asn1` here: [https://luca.ntop.org/Teaching/Appunti/asn1.html](https://luca.ntop.org/Teaching/Appunti/asn1.html).


The protocol uses Elliptic Curve signatures. The curve used is `Ed25519`.


### Concepts

The protocol consists of a series of ledgers, each with two participants, Alice and Bob.

Alice and Bob are identified by their public keys.

The protocol tracks an amount of credit issued by one party to another.

The ledger may also define an amount of collateral that has been pledged against the credit.

Credit and collateral are defined by a unit of account, which is defined for the entire ledger.

Each credit issuance consists of one or more deltas (KeeEntry) changing the balance of the credit, and some identical metadata (KeeEntryHead) embedded in each one, together called an _entry item_.


### Creating a new entry

We call the creator of the entry Alice.

The default values of `signatureRequest` and `signatureResponse` is an empty octet string (length 0).

The default value for `response` is `False`.

If the entry is the first in the ledger, the value of `parent` is 64 0-bytes. If not, it is the sha512 sum of the preceding _entry item_.

Alice generates the _entry item_ and signs it:

```
head = der(KeeEntryHead)
headsum = sha512(head)
entry = der(KeeEntry)
msg = sha512(concat(head, entry)
sig = sign(msg) # TODO verify whether an additional sha512 on the message is performed by the internals
copy(KeeEntry.signatureRequest, sig)
```

### Counter-signing an entry

Alice shares KeeEntryHead and KeeEntry with Bob.

Bob verifies Alice's signature.

If Bob accepts the contents of the _entry item_, he sets `response` to `True`.

Bob proceeds to sign the _entry item_:

```
set(KeeEntry.response, bobs_decision)
head = der(KeeEntryHead)
headsum = sha512(head)
entry = der(KeeEntry)
msg = sha512(concat(head, entry)
sig = sign(msg)
copy(KeeEntry.signatureResponse, sig)
```

Bob then adds the counter-signed _entry item_ to his persistent storage.


### Sharing the result

Bob shares KeeEntryHead and KeeEntry with Alice.

Alice verifies both signatures.

Alice adds the counter-signed _entry_item_ to her persistent storage.

Alice and Bob now have the same ledger state, adjusted by the deltas in the _entry item_.


### Sanity checks

Checking that the delta in an _entry item_ makes sense is the responsibility of both parties, e.g. that the delta does not incur a negative credit value.


### Descriptive content

Both the KeeEntryHead and KeeEntry structures include an optional `body` value, which is a sha512 hash of some content offering some description of what the ledger and individual changes consist of. If no description is provided, 64 0-bytes are used for the data field.

This implementation uses, as previously mentioned, [MIME-structures](https://datatracker.ietf.org/doc/html/rfc2049) for this purpose.


## Data carriers

A final implementation should define several ways of sharing content, including:

* Copy and paste 
* File sharing
* Network retrieval
* QR codes
This implementation uses QR codes to exchange signature requests and signatures.


### QR exchange

A separate transport container, `KeeTransport`, is used for this exchange, aswell as some additional encoding.

```
container = KeeTransport(KeeEntryHead, KeeEntry)
raw = der(container)
compressed = zlib_compress(raw)
payload = base64(compressed)
transport = concat(0x02, readable) # 0x02 = KEE_CMD_LEDGER, defined in src/transport.h
qrencode(transport)
```


#### Sharing descriptive content

Descriptive content is currently not shared as part of the signature exchange.

Since decriptive content can potentially be of some size, a transport protocol should be considered which can span multiple sequential QR codes, and allow of the same kind of offline exchange as the signature exchange itself.


## Examples

The `make testdata` script creates several useful data examples:

- `testdata/mdb` which is the persistent storage of a ledger and required indicies. Can be easily viewed using the `mdb_dump` command. Key prefixes are defined in `src/db.h:enum DbKey`
- `testdata/crypt` contains alice and bob's private and public keys in sexp format, aswell as an encrypted version of the private keys. The encryption used is `CHACHA20-POLY1305`, and password for both keys is `1234`. The `kee.key` symlink points to the private key (Alice) used by the GTK application.
- `testdata/crypt_reverse` contains a symlink `kee.key` to Bob's encrypted private key, enabling use of the GTK application as Bob.
- `testdata/import` Three files respresenting all phases of the _entry item_ format:
    * `request` - without signatures.
    * `response` - with Alice's signature.
    * `final` - with both signatures.
- `testdata/resource` All descriptive content for the generated _ledgers_ and _entry items_.


## License

[GPLv3](https://www.gnu.org/licenses/gpl-3.0.txt)
