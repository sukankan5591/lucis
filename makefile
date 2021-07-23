CC=gcc
CFLAGS=-Wall -std=gnu99 -static
LDFLAGS=-lpthread

all: halo land

halo: halo.o comm_halo.o env_halo.o domain_halo.o
	$(CC) $^ $(LDFLAGS) -o $@
	strip halo

land: land.o comm_land.o
	$(CC) $^ $(LDFLAGS) -o $@
	strip land

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@
	
clean:
	rm -f *.o *.rpm *.deb halo land

