OBJS := $(patsubst %.c,%.o,$(filter-out main.c,$(wildcard *.c)))
INCLUDES := `pkg-config --cflags gtk4 libgcrypt lmdb libxdg-basedir`
CFLAGS +=  $(INCLUDES) -g3 -Wall -Iaux/varint
LIBS := `pkg-config --libs gtk4 libgcrypt zlib lmdb libxdg-basedir` -lb64
LDFLAGS += $(LIBS)

all: aux resource $(OBJS)
	$(CC) $(CFLAGS) main.c -o a.out $(OBJS) $(LDFLAGS) aux/varint/varint.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

resource:
	glib-compile-resources kee.gresource.xml --target=resources.c --generate-source

aux:
	make -C aux

.PHONY: clean aux
