OBJS := $(patsubst %.c,%.o,$(filter-out test.c gen.c,$(wildcard *.c)))
INCLUDES := -I.
CFLAGS += $(INCLUDES)
VERSION = 0.0.1

all: $(OBJS)

test: all
	$(CC) $(CFLAGS) test.c beamenu.o export.o import.o -o test.out $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ $(LDFLAGS)

clean:
	rm -vf *.o
	rm -vf *.out
	rm -vf *.tar.gz
	rm -vf beamenu_gen
	rm -vf beamenu_defs.h

archive:
	git archive --format=tar.gz HEAD -o beamenu-$(VERSION).tar.gz

gen: all
	$(CC) $(CFLAGS) gen.c beamenu.o export.o import.o -o beamenu_gen $(LDFLAGS)
