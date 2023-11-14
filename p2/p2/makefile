.phony all:
all: trains

trains:
	gcc trains.c -o mts -lpthread

.phony gdb:
gdb: trains_gdb

trains_gdb:
	gcc -g trains.c -o mts -lpthread


.PHONY clean:
clean:
	-rm -rf mts