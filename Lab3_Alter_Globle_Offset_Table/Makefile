
CC		= gcc
CFLAGS	= -Wall -g -D_GNU_SOURCE -fPIC
LDFLAGS = -lunwind -lunwind-$(shell uname -m)
PROGS	= chals libpoem.so libsolver.so

all: run #$(PROGS)

%.o: %.c
	$(CC) -c $< $(CFLAGS)

# chals: chals.o libpoem.so
# 	$(CC) -o $@ $^ $(LDFLAGS)

libpoem.so: libpoem.o
	$(CC) -o $@ -shared $^ $(LDFLAGS)

libsolver.so: libsolver.o
	$(CC) -o $@ -shared $^ $(LDFLAGS)

run: libpoem.so libsolver.so chals
	LD_LIBRARY_PATH=`pwd` LD_PRELOAD=./libsolver.so ./chals

cleanup:
	rm -f *.o

clean:
	rm -f $(PROGS) *.o
	rm -rf __pycache__

