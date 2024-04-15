INCLUDES := `pkg-config --cflags libtasn1`
LIBS := `pkg-config --libs libtasn1`
LDFLAGS += $(LIBS)

all: compile

src:
	$(CC) $(CFLAGS) generate.c -o generate $(LDFLAGS)

%.o: %_asn1_tab.c
	$(CC) $< -o $* $(LDFLAGS)

compile: generate schema_entry_asn1_tab.o

generate: src
	./generate

clean:
	rm -vf *.o
	rm -vf *_asn1_tab.c
	rm -vf generate

.PHONY: clean
