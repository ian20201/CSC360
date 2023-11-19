.phony all:
all: disklist diskget diskput diskinfo

disklist: disklist.c
	gcc -o disklist diskstructs.c disklist.c -lm -std=gnu99

diskget: diskget.c
	gcc -o diskget  diskstructs.c diskget.c -lm -std=gnu99

diskput: diskput.c
	gcc -o diskput diskstructs.c diskput.c -lm -std=gnu99

diskinfo: diskinfo.c
	gcc -o diskinfo diskstructs.c diskinfo.c -lm -std=gnu99

.PHONY clean:
clean:
	-rm -rf *.o *.exe disklist diskget diskput diskinfo