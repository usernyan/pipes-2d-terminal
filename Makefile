DEBUGFLAGS += -g -Wall -Wformat -fsanitize=address

pipes2d : pipes2d.c
	gcc -o pipes2d pipes2d.c -DNCURSES_WIDECHAR=1 -lncurses $(DEBUGFLAGS)
