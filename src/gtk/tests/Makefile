# TODO: test files are only generated on second run

OBJS := $(patsubst %.c,%.o,$(wildcard *.c))
LINKOBJS := $(wildcard ../../*.o) $(wildcard ../*.o)
INCLUDES :=  -I../.. -I..
CFLAGS +=  `pkg-config --cflags gtk4  gstreamer-1.0 libqrencode` $(INCLUDES) -Wall
#LIBS := `pkg-config --libs gtk4 zlib lmdb libgcrypt libxdg-basedir  gstreamer-1.0` -lb64 -lvarint -lcmime -llash
LIBS := `pkg-config --libs gtk4 zlib lmdb libgcrypt libxdg-basedir  gstreamer-1.0  libqrencode` -lb64 -lcmime -llash -ltasn1 -lldap
LDFLAGS += $(LIBS) 

all: $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) $< -o test_$* $(LINKOBJS) $(LDFLAGS)

test_run: $(wildcard test_*)
	./$<

test: all test_run

clean:
	rm -vf test_*
