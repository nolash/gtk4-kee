OBJS := $(patsubst %.c,%.o,$(wildcard *.c))
LINKOBJS := $(wildcard ../*.o)
INCLUDES :=  -I.. -I../aux/include
CFLAGS +=  `pkg-config --cflags zbar` $(INCLUDES) -Wall -DRERR -DRERR_N_PREFIX=2
LIBS := `pkg-config --libs zlib lmdb libgcrypt libxdg-basedir libqrencode zbar` -lb64 -llash -ltasn1 -lcmime -lldap -L../aux/lib
LDFLAGS += $(LIBS) 

all: $(OBJS)
	gcc $(CFLAGS) $(LIBS) $(LINKOBJS) -o kee-$@ $<
