all: subs

subs: glade
	make -C src

glade:
	gtk4-builder-tool simplify --3to4 glade.ui > src/main.ui

clean:
	rm -vf src/*.o
	rm -vf src/a.out

run: all
	G_MESSAGES_DEBUG=all  ./src/a.out

debug: all
	G_DEBUG=3 G_MESSAGES_DEBUG=all ./src/a.out
