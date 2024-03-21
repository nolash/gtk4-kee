OBJS := $(patsubst %.c,%.o,$(wildcard *.c))
LINKOBJS := $(wildcard ../../*.o) $(wildcard ../*.o) ../../aux/varint/varint.o
INCLUDES :=  -I../.. -I..
CFLAGS +=  `pkg-config --cflags gtk4  gstreamer-1.0` $(INCLUDES) -g3 -Wall
LIBS := `pkg-config --libs gtk4 zlib lmdb libgcrypt libxdg-basedir  gstreamer-1.0` -lb64 
LDFLAGS += $(LIBS) 

all: $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) $< -o test_$* $(LINKOBJS) $(LDFLAGS)

test_run: $(wildcard test_*)
	./$<

test: all test_run

clean:
	rm -vf test_*
