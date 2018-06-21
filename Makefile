CC=gcc
CFLAGS=
LDFLAGS=-lseccomp
EXEC=mem_limit demo

all: $(EXEC)

mem_limit: 
	$(CC) $(CFLAGS) $@.c -o $@ $(LDFLAGS)

demo:
	$(CC) $(CFLAGS) $@.c $(LDFLAGS) -o $@ $(LDFLAGS)

.PHONY: clean mrproper

clean:
	@rm -f *.o

mrproper: clean
	@rm -f $(EXEC)
