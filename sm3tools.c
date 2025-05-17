#include <stdio.h>
#include <string.h>

enum FILE_TYPES {
    PCPACK,
    PCSSB
};

enum FILE_TYPES getFileType(const char* fileName) {
    char *const fileExtension = strrchr(fileName, '.');
    //printf("%s", fileExtension);

    // TODO could check magic numbers as well

    // FIXME case sensitive currently

    if (strcmp(fileExtension, ".PCPACK") == 0) return PCPACK;
    if (strcmp(fileExtension, ".pcssb") == 0) return PCSSB;
}

int main(int argc, char* argv[]) {
    getFileType(argv[1]);

    return 0;
}
