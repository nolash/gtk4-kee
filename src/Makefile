OBJS := $(patsubst %.c,%.o,$(filter-out main.c,$(wildcard *.c)))
INCLUDES := `pkg-config --cflags libgcrypt lmdb libxdg-basedir`
#CFLAGS +=  $(INCLUDES) -g3 -Wall -Iaux/varint
CFLAGS +=  $(INCLUDES) -Wall
LIBS := `pkg-config --libs libgcrypt zlib lmdb libxdg-basedir` -lb64 -lvarint -llash
LDFLAGS += $(LIBS)

#all: aux resource $(OBJS)
#	$(CC) $(CFLAGS) main.c -o a.out $(OBJS) $(LDFLAGS) aux/varint/varint.o
#all: aux $(OBJS)
all: asn $(OBJS)

asn:
	make -C asn1 compile
	cp -v asn1/*.o .

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

#aux:
#	make -C aux

clean:
	rm -vf *.o
	make -C gtk clean
	make -C asn1 clean
	make -C tests clean
#	make -C aux clean

#.PHONY: clean aux
.PHONY: clean
