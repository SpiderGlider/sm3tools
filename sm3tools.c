#include <assert.h>
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
    assert(fileName != NULL);

    const char *const fileExtension = strrchr(fileName, '.');

    if (fileExtension == NULL) {
        fprintf(stderr, "ERROR: Argument doesn't have a file extension."
                        "Are you sure this is a path to a file?\n");
        exit(EXIT_FAILURE);
    }

    // TODO could check magic numbers as well
    // FIXME case sensitive currently
    if (strcmp(fileExtension, ".PCPACK") == 0) return PCPACK;
    if (strcmp(fileExtension, ".pcssb") == 0) return PCSSB;

    fprintf(stderr, "ERROR: File extension not recognised.\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "ERROR: Must have 1 argument.\n");
        exit(EXIT_FAILURE);
    }
    if (argc > 2) {
        fprintf(stderr, "WARNING: Arguments after the 1st"
                        " argument are currently ignored.\n");
    }

    FileType fileType = getFileType(argv[1]);
    if (fileType == PCPACK) {
        fprintf(stderr, "ERROR: PCPACK parsing is not yet implemented.\n");
    }
    // currently getFileType should never return a value outside of these two
    assert(fileType == PCSSB);



    return EXIT_SUCCESS;
}
