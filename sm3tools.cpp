#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

typedef enum FILE_TYPE {
    PCPACK,
    PCSSB
} FileType;

// Checks the file extension of the file name, and returns the relevant
// file type if one is matched. Otherwise it exits with an error message
FileType getFileType(const char *const fileName) {
    assert(fileName != nullptr);

    const char *const fileExtension = strrchr(fileName, '.');

    if (fileExtension == nullptr) {
        (void) fprintf(stderr, "ERROR: Argument doesn't have a file extension."
                        " Are you sure this is a path to a file?\n");
        exit(EXIT_FAILURE);
    }

    // TODO could check magic numbers as well
    // FIXME case sensitive currently
    if (strcmp(fileExtension, ".PCPACK") == 0) return PCPACK;
    if (strcmp(fileExtension, ".pcssb") == 0) return PCSSB;

    (void) fprintf(stderr, "ERROR: File extension not recognised.\n");
    exit(EXIT_FAILURE);
}

// Program takes one argument, that being the path to a file to parse.
// Currently only PCSSB parsing is implemented. The file type is determined
// only through the file extension currently.
int main(const int argc, const char *const argv[]) {
    if (argc < 2) {
        (void) fprintf(stderr, "ERROR: Must have 1 argument"
                        " (the path to a file to parse).\n");
        exit(EXIT_FAILURE);
    }
    if (argc > 2) {
        (void) fprintf(stderr, "WARNING: Arguments after the 1st"
                        " argument are currently ignored.\n");
    }

    const FileType fileType = getFileType(argv[1]);
    if (fileType == PCPACK) {
        (void) fprintf(stderr, "ERROR: PCPACK parsing is not yet implemented.\n");
        exit(EXIT_FAILURE);
    }
    // currently getFileType should never return a value outside of these two
    assert(fileType == PCSSB);
    (void) printf("INFO: Parsing as a PCSSB file.\n");

    return EXIT_SUCCESS;
}
