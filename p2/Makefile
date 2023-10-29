.phony all:
all: mtrain

mtrain:
	gcc mtrain.c -o A2 -lpthread

.phony gdb:
gdb: mtrain_gdb

mtrain_gdb:
	gcc -g mtrain.c -o A2 -lpthread


.PHONY clean:
clean:
	-rm -rf A2