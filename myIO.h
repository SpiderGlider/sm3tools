#ifndef MYIO_H
#define MYIO_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdio.h>

//cross-platform mkdir wrapper.
//creates directory with mode 0777 on linux
//logs the error if the directory failed to be created
void mymkdir(const char *const path);

//cross-platform stat wrapper.
//logs the error and exits if performing stat fails
intmax_t getfilesize(const char *const path);

//wrapper functions around the <stdio.h> I/O functions
//with additional logging/checks

//wrapper around fopen that checks if the returned
//pointer is NULL, in which case it prints the error to stderr and exits.
//NOTE: this does not close the file handle so you have to call fclose
//after you're done using it, like with normal fopen.
FILE *myfopen(const char *fileName, const char *mode);

//wrapper around fread that checks feof and ferror
//after doing so. Prints to the terminal if any of those happen, and exits
//in the case of ferror. The number of objects read is checked to see whether
//it matches count, but it is still returned in case the caller wants to use it
//for e.g. loop conditions
size_t myfread(
    void *const buffer,
    const size_t size,
    const size_t count,
    FILE *const stream);

//wrapper around fwrite that checks ferror after calling it.
//if there is an error it is printed and then the program exits.
//The number of objects written is checked to see whether
//it matches count, but it is still returned in case the caller wants to use it
//for e.g. loop conditions
size_t myfwrite(
    void *const buffer,
    const size_t size,
    const size_t count,
    FILE *const stream);

//wrapper around fseek that checks whether the return
//value is non-zero, in which case it prints to stderr and exits.
void myfseek(FILE *const stream, const long int offset, const int origin);

//myfseek but working with unsigned long values. accounts for values over what
//signed longs support by seeking twice.
//if first fseek fails second isn't executed.
void myfseek_unsigned(FILE *const stream, const unsigned long int offset, const int origin);
#ifdef __cplusplus
}
#endif
#endif
