#include "myIO.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <direct.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

void mymkdir(const char *const path) {
#ifdef _WIN32
    const int returnValue = _mkdir(path);
#else
    const int returnValue = mkdir(path, 0700);
#endif
    if (returnValue != 0) {
        perror("ERROR: Failed to create directory");
    }
}

intmax_t getfilesize(const char *const filePath) {
#ifdef _WIN32
    _stat64 sb;
    const int returnValue = _wstat64(filePath, &sb);
    const intmax_t size = (intmax_t) sb.st_size;
#else
    struct stat sb;
    const int returnValue = stat(filePath, &sb);
    const intmax_t size = (intmax_t) sb.st_size;
#endif
    if (returnValue != 0) {
        perror("ERROR: Failed to get file size");
        exit(EXIT_FAILURE);
    }
    if (size < 0) {
        fprintf(stderr, "WARNING: File size is negative,"
                        "it may or may not be handled correctly.\n");
    }
    return size;
}

FILE *myfopen(const char *fileName, const char *mode) {
    FILE *const fileHandle = fopen(fileName, mode);
    if (!fileHandle) {
        perror("ERROR: Failed to open file");
        exit(EXIT_FAILURE);
    }
    return fileHandle;
}

size_t myfread(
    void *const buffer,
    const size_t size,
    const size_t count,
    FILE *const stream) {

    //TODO will want to silence these kinds of logs for production...
    if (size == 0) {
        (void) printf("LOG: size of 0 was passed to fread!\n");
    }
    if (count == 0) {
        (void) printf("LOG: count of 0 was passed to fread!\n");
    }

    const size_t objsRead = fread(buffer, size, count, stream);

    if (ferror(stream)) {
        perror("ERROR: I/O error when reading");
        (void) fclose(stream);
        exit(EXIT_FAILURE);
    }
    if (feof(stream)) {
        (void) printf("LOG: EOF encountered when reading file.\n");
    }
    if (objsRead < count) {
        (void) printf("LOG: count was %lu, amount read was only %lu.\n", count, objsRead);
    }

    return objsRead;
}

size_t myfwrite(
    void *const buffer,
    const size_t size,
    const size_t count,
    FILE *const stream) {

    const size_t objsWritten = fwrite(buffer, size, count, stream);

    if (ferror(stream)) {
        perror("ERROR: I/O error when writing");
        (void) fclose(stream);
        exit(EXIT_FAILURE);
    }
    if (objsWritten < count) {
        (void) printf("LOG: count was %lu, amount written was only %lu.\n", count, objsWritten);
    }

    return objsWritten;
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
            myfseek(stream, (long int) -(offset - LONG_MAX), SEEK_CUR);
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
