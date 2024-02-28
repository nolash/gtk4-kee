OBJS := $(patsubst %.c,%.o,$(filter-out main.c,$(wildcard *.c)))
LINKOBJS := $(wildcard ../*.o) $(OBJS) ../aux/varint/varint.o
INCLUDES := `pkg-config --cflags gtk4` -I..
CFLAGS +=  $(INCLUDES) -g3 -Wall
LIBS := `pkg-config --libs gtk4 zlib lmdb libgcrypt` -lb64
LDFLAGS += $(LIBS)

all: resource $(OBJS)
	$(CC) $(CFLAGS) main.c -o a.out $(LINKOBJS) $(LDFLAGS) 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

debug: all

clean:
	rm -vf *.o
	rm -vf a.out
	rm -vf resources.c

resource:
	glib-compile-resources kee.gresource.xml --target=resources.c --generate-source
