CFLAGS=-O2 -Wall
EXE=11int 12str 21int_ens 23large_ens

all:$(EXE)

11int:11int.c ../khashl.h
	$(CC) $(CFLAGS) -I.. -o $@ $<

12str:12str.c ../khashl.h
	$(CC) $(CFLAGS) -I.. -o $@ $<

21int_ens:21int_ens.c ../khashl.h
	$(CC) $(CFLAGS) -I.. -o $@ $<

23large_ens:23large_ens.c ../khashl.h
	$(CC) $(CFLAGS) -I.. -o $@ $<

clean:
	rm -f $(EXE)
