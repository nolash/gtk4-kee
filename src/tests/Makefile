# TODO: test files are only generated on second run
#
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))
LINKOBJS := $(wildcard ../*.o)
INCLUDES :=  -I..
CFLAGS +=  `pkg-config --cflags gtk4  gstreamer-1.0` $(INCLUDES) -g3 -Wall
LIBS := `pkg-config --libs gtk4 zlib lmdb libgcrypt libxdg-basedir  gstreamer-1.0` -lb64 -lvarint
LDFLAGS += $(LIBS) 

all: obj_debug $(OBJS)

obj_debug:
	$(CC) $(CFLAGS) -c debug.c -o debug.o 

%.o: %.c
	$(CC) $(CFLAGS) $< -o test_$* debug.o $(LINKOBJS) $(LDFLAGS)

test_run: $(wildcard test_*)
	./$<

test: all test_run

clean:
	rm -vf test_*
