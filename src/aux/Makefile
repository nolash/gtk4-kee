all:
	make -C llog
	make -C rerr
	make -C beamenu
	make -C beamenu gen

local:
	-make -f Makefile.local

clean:
	make -C llog clean
	make -C rerr clean
	make -C beamenu clean

.PHONY: clean
