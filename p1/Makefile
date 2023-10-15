.phony all:
all: threeone

threeone:
	gcc p1.c -lreadline -lhistory -ltermcap -o A1

.phony gdb:
gdb: threeone_gdb

threeone_gdb:
	gcc -g p1.c -lreadline -lhistory -ltermcap -o A1


.PHONY clean:
clean:
	-rm -rf A1