.phony all:
all: trains

trains:
	gcc trains.c -o A2 -lpthread

.phony gdb:
gdb: trains_gdb

trains_gdb:
	gcc -g trains.c -o A2 -lpthread


.PHONY clean:
clean:
	-rm -rf A2