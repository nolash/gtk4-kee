OBJS := $(patsubst %.c,%.o,$(filter-out main.c,$(wildcard *.c)))
INCLUDES := `pkg-config --cflags libgcrypt lmdb libxdg-basedir`
CFLAGS +=  $(INCLUDES) -g3 -Wall -Iaux/varint
LIBS := `pkg-config --libs libgcrypt zlib lmdb libxdg-basedir` -lb64
LDFLAGS += $(LIBS)

#all: aux resource $(OBJS)
#	$(CC) $(CFLAGS) main.c -o a.out $(OBJS) $(LDFLAGS) aux/varint/varint.o
all: aux $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

aux:
	make -C aux

clean:
	rm -vf *.o
	make -C gtk clean
	make -C aux clean

dev:
	$(CC) $(CFLAGS) -c settings.c -o settings.o $(LDFLAGS)
	$(CC) $(CFLAGS) -c context.c -o context.o $(LDFLAGS)
	$(CC) $(CFLAGS) -c camera.c -o camera.o $(LDFLAGS)
	$(CC) $(CFLAGS) -c state.c -o state.o
	make -C gtk dev

.PHONY: clean aux
