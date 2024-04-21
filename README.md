# kee

I am building a cryptographically counter-signed IOU application with linux phones ([GTK4](https://www.gtk.org/), [phosh](https://phosh.mobi/)) as the primary targets.

It will maintain bi-lateral mutual channels of credit and collateral. Each change is linked as a chain, and documentation for each change in embedded as [email-link MIME structures](https://datatracker.ietf.org/doc/html/rfc2049)


## Dependencies

Below are the library versions of the [archlinux](https://archlinux.org/) component packages I am using for dependencies and tooling while working on my own system.

| package | version |
|---|---|
| gcc | 13.2.1 |
| gst-plugins-bad  | 1.24.0 |
| gstreamer | 1.24.0 |
| gtk4 | 4.12.5 |
| libb64 | 1.2.1 |
| libgcrypt | 1.10.3 |
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
| liblash | [https://holbrook.no/src/liblash](https://holbrook.no/src/liblash) `(193dd4032e8088ada2dc9dcedab8d7486e9305f5)` | `0.1.0` | `333cdf49bb7e9f44b37e5bc9f594a01e4b3b8ecb3fcd3d9ffec3ea6dcdeaec7b` |
| libcmime | [https://github.com/spmfilter/libcmime.git](https://github.com/spmfilter/libcmime.git) `(dd21eb096d162656e30243f60fc4bc35ad39ae6e)` | `0.2.2` | `18a8d46ebec575a79212acc2dc6af7fd7bdeba3a9b85a70677ed0b7785c5c04e` |
| gst-plugins-rs | [https://gitlab.freedesktop.org/gstreamer/gst-plugins-rs](https://gitlab.freedesktop.org/gstreamer/gst-plugins-rs) `(a84bbc66f30573b62871db163c48afef75adf6ec)` | `1.22.10 (gstreamer)` | `691d5d52f59ec6322a2f6ddc1039ef47c9ac5e6328e2df1ef920629b46c659df` |


## Building

There is no magic pathfinder implemented currently. Thus, you must configure the environment for building and linking the external sources yourself.

Here is what I use in my local setup.


### varint

```
git clone https://github.com/tidwall/varint.c
cd varint.c
git checkout 7e1f7067eaf6ee114d865c99a8f5f01c813f3a61
gcc -c varint.c
gcc -shared -o libvarint.so varint.o
``` 

The repository root directory will be your include and ld path.


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
export GST_PLUGIN_PATH=/home/lash/src/build/gst-plugins-rs/0.9.13/target/x86_64-unknown-linux-gnu/debug/
export LIBRARY_PATH=$LIBRARY_PATH:/home/lash/src/build/varint.c/0.1.0:/home/lash/src/build/libcmime/0.2.2/build/usr/local/lib64
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/lash/src/build/varint.c/0.1.0:/home/lash/src/build/libcmime/0.2.2/build/usr/local/lib64
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/home/lash/src/build/libcmime/0.2.2/build/usr/local/lib64/pkgconfig
export C_INCLUDE_PATH=$C_INCLUDE_PATH:/home/lash/src/build/varint.c/0.1.0:/home/lash/src/build/libcmime/0.2.2/build/usr/local/include

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
