CFLAGS=-O2 -std=c99 -Wall -Wc++-compat #-fsanitize=address,undefined
EXE=test_int test_str

all:$(EXE)

test_int:test_int.c khashp.c khashp.h
	$(CC) $(CFLAGS) -o $@ $< khashp.c

test_str:test_str.c khashp.c khashp.h
	$(CC) $(CFLAGS) -o $@ $< khashp.c

clean:
	rm -f $(EXE)
