#include "pcssb.h"

#include <stdio.h>

struct FSB readFile(const char* fileName) {
    struct FSB fsb = {};
    FILE *fileHandle = fopen(fileName, "rb");

    const size_t count = 100;
    uint32_t buffer[100] = {};

    const size_t numRead = fread(buffer, 4, count , fileHandle);
    if (numRead < count) {
        printf("LOG: count was %lu, amount read was only %lu.\n", count, numRead);
    }

    fsb.fsb3Header = buffer[0];
    fsb.numFiles = buffer[1];
    fsb.unknown1 = buffer[2];
    fsb.dataSize = buffer[3];
    fsb.unknown2 = buffer[4];
    fsb.null1 = buffer[5];

    fclose(fileHandle);

    return fsb;
}

int main(int argc, char* argv[]) {
    const struct FSB fsb = readFile(argv[1]);
    printf("%s", (char*) &fsb.fsb3Header);
}

