shell: hw3.c
	gcc -g -Wall -Wvla -fsanitize=address -o hw3 hw3.c

clean: shell
	rm -f shell