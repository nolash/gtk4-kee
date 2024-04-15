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
	G_DEBUG=3 G_MESSAGES_DEBUG=all ./src/gtk/a.out

debug: gtk all
	G_DEBUG=all G_MESSAGES_DEBUG=all gdb ./src/gtk/a.out

#test: gtk all test_src test_gtk
test: testdata test_src test_gtk

testdata_schema:
	asn1ate src/asn1/schema_entry.txt > testdata_asn1schema.py

test_src: all
	make -C src/tests test

test_gtk: gtk all
	make -C src/gtk/tests test

testdata: testdata_schema
	rm -vrf testdata_mdb
	#python testdata.py
	python testdata_asn1.py

doc:
	pandoc -fgfm -tplain README.md > README
