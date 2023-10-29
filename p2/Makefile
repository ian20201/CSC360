.phony all:
all: threeone

threeone:
	gcc mtrain.c -o A2 -lpthread

.phony gdb:
gdb: threeone_gdb

threeone_gdb:
	gcc -g mtrain.c -o A2 -lpthread


.PHONY clean:
clean:
	-rm -rf A2