OBJS := $(patsubst %.c,%.o,$(filter-out cli.c,$(wildcard *.c)))
LINKOBJS := $(wildcard ../*.o)
INCLUDES :=  -I.. -I../aux/include
CFLAGS +=  `pkg-config --cflags zbar` $(INCLUDES) -Wall -DRERR -DRERR_N_PREFIX=2
LIBS := `pkg-config --libs zlib lmdb libgcrypt libxdg-basedir libqrencode zbar` -lb64 -llash -ltasn1 -lcmime -lldap -L../aux/lib
LDFLAGS += $(LIBS) 

all: $(OBJS) cliobj
	gcc $(CFLAGS) $(LIBS) $(LINKOBJS) cli.o -o kee-$@ $<

cliobj:
	gcc $(CFLAGS) -c cli.c -o cli.o
