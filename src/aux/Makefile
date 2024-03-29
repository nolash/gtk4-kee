# TODO: unify destinations


all: varint cmime

varint:
	make -C varint

cmime:
	mkdir -p libcmime/build
	cd libcmime/build && cmake ..
	make -C libcmime/build
	mkdir -vp include
	cd include && ln -vsf ../libcmime/src cmime

clean:
	make -C varint clean
	make -C libcmime/build clean

.PHOBY: clean
