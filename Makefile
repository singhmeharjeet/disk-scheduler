all:
	gcc -Wall -Werror -Wextra -g main.c -o DSSimul

clean:
	rm -rf DSSimul *.dSYM
