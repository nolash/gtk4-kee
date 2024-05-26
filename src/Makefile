OBJS := $(patsubst %.c,%.o,$(filter-out main.c,$(wildcard *.c)))
INCLUDES := `pkg-config --cflags libgcrypt lmdb libxdg-basedir libqrencode zbar`
CFLAGS +=  $(INCLUDES) -Wall
LIBS := `pkg-config --libs libgcrypt zlib lmdb libxdg-basedir libqrencode zbar` -lb64 -llash
LDFLAGS += $(LIBS)

#all: aux resource $(OBJS)
#	$(CC) $(CFLAGS) main.c -o a.out $(OBJS) $(LDFLAGS) aux/varint/varint.o
all: asn aux $(OBJS)

asn:
	make -C asn1 compile
	cp -v asn1/*.o .

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

aux:
	echo "foo"
	make -C aux all

clean:
	rm -vf *.o
	make -C gtk clean
	make -C asn1 clean
	make -C tests clean
	make -C aux clean

.PHONY: clean aux
