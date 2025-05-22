#ifndef MYIO_H
#define MYIO_H
#include <stdio.h>

//my version of fopen that calls the stdio one but checks if the returned
//pointer is NULL. If it is it prints the error to stderr and exits.
FILE *myfopen(const char *fileName, const char *mode);

//my version of fread that calls the stdio one but checks feof and ferror
//after doing so. Prints to the terminal if any of those happen, and exits
//in the case of ferror. The number of objects read is checked to see whether
//it matches count, but it is still returned in case the caller wants to use it
//for e.g. loop conditions
size_t myfread(
    void *const buffer,
    const size_t size,
    const size_t count,
    FILE *const stream);

//my version of fread that calls the stdio one but checks ferror after doing so.
//if there is an error it  is printed and then the program exits.
//The number of objects written is checked to see whether
//it matches count, but it is still returned in case the caller wants to use it
//for e.g. loop conditions
size_t myfwrite(
    void *const buffer,
    const size_t size,
    const size_t count,
    FILE *const stream);

//my version of fseek that calls the stdio one but checks whether the return
//value is non-zero. If it is it prints to stderr and exits.
void myfseek(FILE *const stream, const long int offset, const int origin);

//myfseek but working with unsigned long values. accounts for values over what
//signed longs support by seeking twice.
//if first fseek fails second isn't executed.
void myfseek_unsigned(FILE *const stream, const unsigned long int offset, const int origin);

#endif
