# TODO: test files are only generated on second run
#
OBJS := $(patsubst %.c,%.o,$(filter-out util.c,$(wildcard *.c)))
LINKOBJS := $(wildcard ../*.o)
INCLUDES :=  -I..
CFLAGS +=  `pkg-config --cflags gtk4  gstreamer-1.0 zbar` $(INCLUDES) -Wall
LIBS := `pkg-config --libs gtk4 zlib lmdb libgcrypt libxdg-basedir gstreamer-1.0 libqrencode zbar` -lb64 -llash -ltasn1 -lcmime -lldap
LDFLAGS += $(LIBS) 

all: obj_debug $(OBJS)

util:
	$(CC) $(CFLAGS) -c testutil.c -o testutil.o 

obj_debug: util
	$(CC) $(CFLAGS) -c debug.c -o debug.o 

%.o: %.c
	$(CC) $(CFLAGS) $< -o test_$* debug.o testutil.o $(LINKOBJS) $(LDFLAGS)

#test_run: $(wildcard test_*)
#	./$<

testdata:
	ln -sf ../../testdata testdata

test_run: testdata
	./test_cadir
	./test_content
	./test_sign
	./test_ledger
	./test_dn
	./test_transport

test: all test_run

clean:
	rm -vf test_*
	rm -vf *.o
