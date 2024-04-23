# TODO: test files are only generated on second run
#
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))
LINKOBJS := $(wildcard ../*.o)
INCLUDES :=  -I..
CFLAGS +=  `pkg-config --cflags gtk4  gstreamer-1.0` $(INCLUDES) -Wall
#LIBS := `pkg-config --libs gtk4 zlib lmdb libgcrypt libxdg-basedir  gstreamer-1.0` -lb64 -lvarint -llash
LIBS := `pkg-config --libs gtk4 zlib lmdb libgcrypt libxdg-basedir  gstreamer-1.0` -lb64 -llash -ltasn1 -lcmime -lldap
LDFLAGS += $(LIBS) 

all: obj_debug $(OBJS)

obj_debug:
	$(CC) $(CFLAGS) -c debug.c -o debug.o 

%.o: %.c
	$(CC) $(CFLAGS) $< -o test_$* debug.o $(LINKOBJS) $(LDFLAGS)

#test_run: $(wildcard test_*)
#	./$<
test_run:
	./test_cadir
	./test_content
	./test_ledger
	./test_dn
	./test_sign

test: all test_run

clean:
	rm -vf test_*
