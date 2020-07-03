CC=gcc
CFLAGS=-Wall -std=gnu99 -pthread -static

all: halo land

halo: halo.o comm_halo.o env_halo.o domain_halo.o
	$(CC) $(CFLAGS) $^ -o $@
	strip halo

land: land.o comm_land.o
	$(CC) $(CFLAGS) $^ -o $@
	strip land

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@
	
clean:
	rm -f *.o *.rpm halo land

