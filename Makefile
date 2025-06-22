.PHONY: clean all

default: bin/sm3tools
all: bin/sm3tools

#UARCH = $(shell uname -m)

DEFAULTFLAGS = -std=c++17 -Wall -pedantic -g -fsanitize=undefined -fsanitize=address

EXTRAFLAGS := -Wextra -Wformat=2 -Wconversion \
 -Wno-unused-parameter -Wshadow -Wfloat-equal -Wundef \
 -Wold-style-cast -Wcast-align -Wsign-conversion -Wdouble-promotion \
 -Wimplicit-fallthrough -Wnon-virtual-dtor # -O3 -Wwrite-strings -Wformat-signedness

ifdef CLANG
	CC ?= clang
	C++ ?= clang++
else
	CC = gcc
	C++ = g++
    GCCFLAGS = -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op \
     -Wnull-dereference -Wuseless-cast
endif

bin/sm3tools: src/sm3tools.cpp src/pcssb.cpp src/myIO.cpp
	$(C++) $(DEFAULTFLAGS) $(EXTRAFLAGS) $(GCCFLAGS) $^ -o $@

%: %.cpp
	$(C++) $(DEFAULTFLAGS) $(EXTRAFLAGS) $@.cpp -o $@

clean:
	rm -f bin/sm3tools
