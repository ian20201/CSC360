.phony all:
all: threeone

threeone:
	gcc 3_1\ modification.c -lreadline -o A1

.phony gdb:
gdb: threeone_gdb

threeone_gdb:
	gcc -g 3_1\ modification.c -lreadline -o A1


.PHONY clean:
clean:
	-rm -rf A1