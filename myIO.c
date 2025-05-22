#include "myIO.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

FILE *myfopen(const char *fileName, const char *mode) {
    FILE *const fileHandle = fopen(fileName, mode);
    if (!fileHandle) {
        perror("ERROR: Failed to open file");
        exit(EXIT_FAILURE);
    }
    return fileHandle;
}

void myfseek(FILE *const stream, const long int offset, const int origin) {
    const int returnValue = fseek(stream, offset, origin);
    if (returnValue != 0) {
        fprintf(stderr, "ERROR: fseek() failed!\n");
        (void) fclose(stream);
        exit(EXIT_FAILURE);
    };
}

// credit to Tyler Durden on SO for this code, which is used with modifications.
// source: https://stackoverflow.com/a/47740105
// licensed under CC BY-SA 3.0
void myfseek_unsigned(FILE *const stream, const unsigned long int offset, const int origin) {
    if (offset > LONG_MAX){
        //call fseek with max value it supports for the offset
        myfseek(stream, LONG_MAX, origin);
        if (origin == SEEK_END) {
            //seeks backwards the remaining distance
            myfseek(stream, -(offset - LONG_MAX), SEEK_CUR);
        }
        else {
            //seeks forward the remaining distance
            myfseek(stream, (long int)(offset - LONG_MAX), SEEK_CUR);
        }
    }
    else {
        //fseek normally if below max supported value
        myfseek(stream, (long int) offset, origin);
    }
}
