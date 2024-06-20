CFLAGS += -I./include

all: prep
	make -C liblash
	make -C beamenu
	make -C beamenu gen

install: all
	make -C liblash install DESTDIR=`realpath .`
	install -m0644 -t include/ beamenu/*.h

local:
	-make -f Makefile.local

clean:
	make -C liblash clean
	make -C beamenu clean

.PHONY: clean

prep:
	mkdir -vp lib
	mkdir -vp include
