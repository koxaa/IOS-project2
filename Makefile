CC=gcc
CFLAGS=-std=gnu99 -Werror -Wextra -Wall -pedantic -g -D_XOPEN_SOURCE=500 # Last switcher is for ulseep(). It doesn't work without this 
	
proj2: 	proj2.c
	$(CC) $(CFLAGS)  proj2.c -o proj2 -lrt -pthread
