# TODO: test files are only generated on second run

OBJS := $(patsubst %.c,%.o,$(wildcard *.c))
LINKOBJS := $(wildcard ../../*.o) $(wildcard ../*.o)
INCLUDES :=  -I../.. -I..
CFLAGS +=  `pkg-config --cflags gtk4  gstreamer-1.0 libqrencode` $(INCLUDES) -Wall
#LIBS := `pkg-config --libs gtk4 zlib lmdb libgcrypt libxdg-basedir  gstreamer-1.0` -lb64 -lvarint -lcmime -llash
LIBS := `pkg-config --libs gtk4 zlib lmdb libgcrypt libxdg-basedir  gstreamer-1.0  libqrencode` -L../../aux/lib -lb64 -lcmime -llash -ltasn1 -lldap
LDFLAGS += $(LIBS) 
#AUXLIBS := `pkg-config --libs kee`

all: $(OBJS)

%.o: %.c
#	$(CC) $(CFLAGS) $< -o test_$* $(LINKOBJS) $(LDFLAGS) $(AUXLIBS)
	$(CC) $(CFLAGS) $< -o test_$* $(LINKOBJS) $(LDFLAGS) ../../aux/beamenu/beamenu.o  ../../aux/beamenu/import.o

test_run: $(wildcard test_*)
	./$<

test: all test_run

clean:
	rm -vf test_*
