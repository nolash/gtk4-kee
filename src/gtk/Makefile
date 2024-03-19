OBJS := $(patsubst %.c,%.o,$(filter-out main.c,$(wildcard *.c)))
LINKOBJS := $(wildcard ../*.o) $(OBJS) ../aux/varint/varint.o
INCLUDES :=  -I..
CFLAGS +=  `pkg-config --cflags gtk4  gstreamer-1.0` $(INCLUDES) -g3 -Wall
LIBS := `pkg-config --libs gtk4 zlib lmdb libgcrypt libxdg-basedir  gstreamer-1.0` -lb64 
LDFLAGS += $(LIBS) 

all: resource $(OBJS)
	$(CC) $(CFLAGS) main.c -o a.out $(LINKOBJS) $(LDFLAGS) 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

debug: all

clean:
	rm -vf *.o
	rm -vf a.out
	rm -vf a_dev.out
	rm -vf resources.c

resource:
	glib-compile-resources kee.gresource.xml --target=resources.c --generate-source

dev:
	$(CC) $(CFLAGS) -c debug.c -o debug.o
	$(CC) $(CFLAGS) -c view.c -o view.o
	$(CC) $(CFLAGS) -c nav.c -o nav.o
	$(CC) $(CFLAGS) -c scan.c -o scan.o
	$(CC) $(CFLAGS) -c ui.c -o ui.o
	$(CC) $(CFLAGS) -c menu.c -o menu.o
	$(CC) $(CFLAGS) -c kee-uicontext.c -o kee-uicontext.o
	$(CC) $(CFLAGS) main.c -o a_dev.out ../settings.o ../camera.o ../context.o ../state.o ./kee-uicontext.o ./debug.o ./menu.o ./scan.o ./ui.o ./view.o ./nav.o $(LDFLAGS)
