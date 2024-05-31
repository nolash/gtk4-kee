CFLAGS += -I./include

all: prep
	make -C liblash install DESTDIR=`realpath .`
	make -C beamenu
	make -C beamenu gen

local:
	-make -f Makefile.local

clean:
	make -C liblash clean
	make -C beamenu clean

.PHONY: clean

prep:
	mkdir -vp lib
	mkdir -vp include
