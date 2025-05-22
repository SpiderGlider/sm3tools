#include "myIO.h"

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
