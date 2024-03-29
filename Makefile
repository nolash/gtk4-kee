all: subs

core: subs

gtk: core
	make -C src/gtk

subs: glade
	make -C src

glade:
	gtk4-builder-tool simplify --3to4 glade.ui > src/gtk/main.ui

clean:
	make -C src clean

run: gtk all
	G_MESSAGES_DEBUG=all ./src/gtk/a.out

debug: gtk all
	G_DEBUG=all G_MESSAGES_DEBUG=all ./src/gtk/a.out

#test: gtk all test_src test_gtk
test: test_src

test_src: all
	make -C src/tests test

test_gtk: gtk all test_src 
	make -C src/gtk/tests test

testdata:
	rm -vrf testdata_mdb
	python testdata.py
