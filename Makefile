all: subs

core: subs

gtk: core
	make -C src/gtk

src: subs

subs: glade
	make -C src all

glade:
	gtk4-builder-tool simplify --3to4 glade.ui > src/gtk/main.ui

clean:
	make -C src clean
	rm -vf testdata_asn1schema.py


run: gtk all
	cd testdata && ln -svf ../src/gtk/beamenu.dat .
	G_DEBUG=3 G_MESSAGES_DEBUG=3 ./src/gtk/a.out

debug: gtk testdata all
	#G_DEBUG=all G_MESSAGES_DEBUG=all ./src/gtk/a.out
	cd testdata && ln -svf ../src/gtk/beamenu.dat .
	G_DEBUG=all G_MESSAGES_DEBUG="Kee Gio Glib" ./src/gtk/a.out

gdb: gtk testdata all
	G_DEBUG=all G_MESSAGES_DEBUG=all gdb ./src/gtk/a.out


#test: gtk all test_src test_gtk
test: test_src test_gtk

testdata_schema:
	asn1ate src/asn1/schema_entry.txt > testdata_asn1schema.py

test_src: all
	make -C src/tests test

test_gtk: testdata gtk all
	make -C src/gtk/tests test

testdata: testdata_schema
	rm -vrf testdata_mdb
	#python testdata.py
	python testdata_asn1.py
	make -C src/tests testdata

testdata_gtk: gtk testdata
	cd testdata && ln -svf ../src/gtk/beamenu.dat beamenu.dat

doc:
	pandoc -fgfm -tplain README.md > README

cmd: src
	make -C src/cmd
