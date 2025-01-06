CFLAGS=-O2 -Wall
EXE=test_int test_str

all:$(EXE)

test_int:test_int.c khashp.c khashp.h
	$(CC) $(CFLAGS) -o $@ $< khashp.c

test_str:test_str.c khashp.c khashp.h
	$(CC) $(CFLAGS) -o $@ $< khashp.c

clean:
	rm -f $(EXE)
