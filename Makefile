.PHONY: clean all

default: pcssb
all: sm3tools pcssb

UARCH = $(shell uname -m)

ifndef USECC
	CC = clang
	C++ = clang++
else
	CC ?= gcc
	C++ ?= g++
endif

DEFAULTFLAGS = -std=c11 -Wall -pedantic -g -fsanitize=undefined -fsanitize=address

DEFAULTFLAGS_CPP = -std=c++17 -Wall -pedantic -g -fsanitize=undefined -fsanitize=address

EXTRAFLAGS = -Wextra -Wformat=2 -Wconversion -Wno-strict-prototypes -Wno-unused-parameter -Wshadow -Wfloat-equal -Wundef # -O3 -Wwrite-strings -Wformat-signedness 

sm3tools: src/sm3tools.cpp
	$(C++) $(DEFAULTFLAGS_CPP) $(EXTRAFLAGS) src/sm3tools.cpp -o bin/sm3tools

pcssb: src/pcssb.cpp
	$(C++) $(DEFAULTFLAGS_CPP) $(EXTRAFLAGS) src/pcssb.cpp src/myIO.cpp -o bin/pcssb

%: %.cpp
	$(C++) $(DEFAULTFLAGS_CPP) $(EXTRAFLAGS) $@.cpp -o $@

clean:
	rm -f bin/sm3tools bin/pcssb
