OBJS := $(patsubst %.c,%.o,$(filter-out cli.c,$(wildcard *.c)))
LINKOBJS := $(wildcard ../*.o)
INCLUDES :=  -I. -I.. -I../aux/include
CFLAGS +=  `pkg-config --cflags libtasn1 zbar` $(INCLUDES) -Wall -DRERR -DRERR_N_PREFIX=2
LIBS := `pkg-config --libs zlib lmdb libgcrypt libxdg-basedir libqrencode zbar` -lb64 -llash -ltasn1 -lcmime -lldap -L../aux/lib
LDFLAGS += $(LIBS) 

all: $(OBJS) cliobj
	$(CC) $(CFLAGS) $(LINKOBJS) cli.o -o kee-$@ $< $(LDFLAGS)

cliobj:
	$(CC) $(CFLAGS) $(LDFLAGS) -c cli.c -o cli.o
