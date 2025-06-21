.PHONY: clean all

default: sm3tools
all: sm3tools

UARCH = $(shell uname -m)

ifndef USECC
	CC = clang
	C++ = clang++
else
	CC ?= gcc
	C++ ?= g++
endif

DEFAULTFLAGS = -std=c++17 -Wall -pedantic -g -fsanitize=undefined -fsanitize=address

EXTRAFLAGS = -Wextra -Wformat=2 -Wconversion \
 -Wno-unused-parameter -Wshadow -Wfloat-equal -Wundef \
 -Wold-style-cast -Wcast-align -Wsign-conversion -Wdouble-promotion \
 -Wimplicit-fallthrough -Wnon-virtual-dtor # -O3 -Wwrite-strings -Wformat-signedness

sm3tools: src/sm3tools.cpp
	$(C++) $(DEFAULTFLAGS) $(EXTRAFLAGS) src/sm3tools.cpp src/pcssb.cpp src/myIO.cpp -o bin/sm3tools

%: %.cpp
	$(C++) $(DEFAULTFLAGS) $(EXTRAFLAGS) $@.cpp -o $@

clean:
	rm -f bin/sm3tools
