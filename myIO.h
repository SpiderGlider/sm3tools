#ifndef MYIO_H
#define MYIO_H
#include <stdio.h>

//my version of fopen that calls the stdio one but checks if the returned
//pointer is NULL. If it is it prints the error to stderr and exits.
FILE *myfopen(const char *fileName, const char *mode);

//my version of fseek that calls the stdio one but checks whether the return
//value is non-zero. If it is it prints to stderr and exits.
void myfseek(FILE *const stream, const long int offset, const int origin);

//myfseek but working with unsigned long values. accounts for values over what
//signed longs support by seeking twice.
//if first fseek fails second isn't executed.
void myfseek_unsigned(FILE *const stream, const unsigned long int offset, const int origin);

#endif
