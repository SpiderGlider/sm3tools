#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum FILE_TYPE {
    PCPACK,
    PCSSB
} FileType;

// Checks the file extension of the file name, and returns the relevant
// file type if one is matched. Otherwise it exits with an error message
FileType getFileType(const char* fileName) {
    const char *const fileExtension = strrchr(fileName, '.');

    // TODO could check magic numbers as well
    // FIXME case sensitive currently

    if (strcmp(fileExtension, ".PCPACK") == 0) return PCPACK;
    if (strcmp(fileExtension, ".pcssb") == 0) return PCSSB;

    fprintf(stderr, "ERROR: File extension not recognised.");
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    FileType fileType = getFileType(argv[1]);

    return EXIT_SUCCESS;
}
