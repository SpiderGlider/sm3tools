#ifndef MYIO_H
#define MYIO_H
#include <stdio.h>

//my version of fopen that calls the stdio one but checks if the returned
//pointer is NULL. If it is it prints the error and exits.
FILE *myfopen(const char *fileName, const char *mode);

#endif
