CFLAGS=-O2
EXE=01int 02str

all:$(EXE)

01int:01int.c ../khashl.h
	$(CC) $(CFLAGS) -I.. -o $@ $<

02str:02str.c ../khashl.h
	$(CC) $(CFLAGS) -I.. -o $@ $<

clean:
	rm -f $(EXE)
