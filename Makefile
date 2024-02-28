all: subs

core: subs

gtk: core
	make -C src/gtk

subs: glade
	make -C src

glade:
	gtk4-builder-tool simplify --3to4 glade.ui > src/main.ui

clean:
	make -C src clean
	make -C src/gtk clean
	rm -vf src/aux/varint/*.o

run: gtk all
	G_MESSAGES_DEBUG=all  ./src/gtk/a.out

debug: gtk all
	G_DEBUG=3 G_MESSAGES_DEBUG=all ./src/gtk/a.out
