.PHONY all:
all:
	gcc -Wall -Wextra -Werror -pedantic-errors diskstructs.c diskinfo.c -o diskinfo.o
	gcc -Wall -Wextra -Werror -pedantic-errors diskstructs.c disklist.c -o disklist.o
	gcc -Wall -Wextra -Werror -pedantic-errors diskstructs.c diskget.c -o diskget.o

.PHONY clean:
clean:
	rm -rf *.o *.exe
