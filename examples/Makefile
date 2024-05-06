CFLAGS=-O2
EXE=11int 12str 21int_ens

all:$(EXE)

11int:11int.c ../khashl.h
	$(CC) $(CFLAGS) -I.. -o $@ $<

12str:12str.c ../khashl.h
	$(CC) $(CFLAGS) -I.. -o $@ $<

21int_ens:21int_ens.c ../khashl.h
	$(CC) $(CFLAGS) -I.. -o $@ $<

clean:
	rm -f $(EXE)
