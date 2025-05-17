#include "pcssb.h"

#include <stdio.h>

struct FSB readFile(const char* fileName) {
    struct FSB fsb = {};
    FILE *fileHandle = fopen(fileName, "rb");

    uint32_t buffer[6] = {};
    fread(buffer, 4, 6, fileHandle);

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
    struct FSB fsb = readFile(argv[1]);
    printf("%u", fsb.unknown1);
}

