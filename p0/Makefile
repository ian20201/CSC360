.phony all:
all: threeone

threeone:
	gcc threeone.c -lreadline -o A1

.phony gdb:
gdb: threeone_gdb

threeone_gdb:
	gcc -g threeone.c -lreadline -o A1


.PHONY clean:
clean:
	-rm -rf A1