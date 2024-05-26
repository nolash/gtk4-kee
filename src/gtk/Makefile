OBJS := $(patsubst %.c,%.o,$(filter-out main.c beamenu_defs.c,$(wildcard *.c)))
LINKOBJS := $(wildcard ../*.o) $(OBJS)
INCLUDES :=  -I.. -I../aux/include
CFLAGS +=  `pkg-config --cflags gtk4  gstreamer-1.0  libtasn1 libqrencode zbar` $(INCLUDES) -g3 -Wall
LIBS := `pkg-config --libs gtk4 zlib lmdb libgcrypt libxdg-basedir gstreamer-1.0 libtasn1 libqrencode zbar` -lb64 -lcmime -llash -lldap
LDFLAGS += $(LIBS) 
AUXLIBS := `pkg-config --libs kee`

all: menu resource $(OBJS)
	$(CC) $(CFLAGS) main.c -o a.out $(LINKOBJS) $(LDFLAGS) $(AUXLIBS) beamenu_defs.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)


debug: all

clean:
	rm -vf *.o
	rm -vf a.out
	rm -vf a_dev.out
	rm -vf resources.c
	rm -vf beamenu_defs.*
	rm -vf beamenu.dat
	make -C tests clean

resource:
	glib-compile-resources kee.gresource.xml --target=resources.c --generate-source

menu:
	../aux/beamenu/beamenu_gen menu.txt
	$(CC) $(CFLAGS) -c beamenu_defs.c -o beamenu_defs.o
