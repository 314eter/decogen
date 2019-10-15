CFLAGS=-O3

OBJECTS=construct.o planar_code.o deco_code.o decogen.o

decogen: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c %.h
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f *.o decogen
