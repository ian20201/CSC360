.phony all:
all: disklist diskget diskput diskinfo

disklist: disklist.c
	gcc -o disklist disklist.c diskstructs.c -lm -std=gnu99

diskget: diskget.c
	gcc -o diskget diskget.c diskstructs.c -lm -std=gnu99

diskput: diskput.c
	gcc -o diskput diskput.c diskstructs.c -lm -std=gnu99

diskinfo: diskinfo.c
	gcc -o diskinfo diskinfo.c diskstructs.c -lm -std=gnu99

.PHONY clean:
clean:
	-rm -rf *.o *.exe disklist diskget diskput diskinfo
