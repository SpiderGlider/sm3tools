.PHONY: clean all

default: sm3tools
all: sm3tools

UARCH = $(shell uname -m)

ifndef USECC
	CC = clang
else
	CC ?= gcc
endif

DEFAULTFLAGS = -std=c11 -Wall -pedantic -g -fsanitize=undefined -fsanitize=address

EXTRAFLAGS = -Wextra -Wformat=2 -Wconversion -Wno-strict-prototypes -Wno-unused-parameter -Wshadow -Wfloat-equal -Wundef # -O3 -Wwrite-strings -Wformat-signedness 

sm3tools: sm3tools.c
	$(CC) $(DEFAULTFLAGS) $(EXTRAFLAGS) sm3tools.c -o $@

%: %.c
	$(CC) $(DEFAULTFLAGS) $(EXTRAFLAGS) $@.c -o $@ 

clean:
	rm -f sm3tools
