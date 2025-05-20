#include "pcssb.h"

#include <assert.h>
#include <stdio.h>

//prints out indexes of every instance of "FSB3" header text in the sound file
//returns number of results found. if it returns a value greater than resultArrLen,
//that means resultArr was too small and there may have been more results that couldn't fit.
size_t findFSBHeaderIndexes(
    const char *const inputFileName,
    char *const resultArr,
    const size_t resultArrLen) {

    //next index in resultArr to fill (0 indexed)
    //also happens to be the number of results currently found
    size_t resultCount = 0;

    FILE *const fileHandle = fopen(inputFileName, "rb");

    const size_t buffSize = 100;
    //magic number needs to be reused to avoid Variable Length Array
    char buffer[100] = {};
    assert(sizeof(buffer) == buffSize);

    const size_t numRead = fread(buffer, 1, buffSize , fileHandle);
    if (numRead < buffSize) {
        printf("LOG: count was %lu, amount read was only %lu.\n", buffSize, numRead);
    }
    assert(numRead == buffSize);

    fclose(fileHandle);

    //string to match in the file (except for null termination)
    const char fsbHeaderString[5] = "FSB3";
    //represents current character in the string to match
    int fsbStrIndex = 0;
    for (size_t i = 0; i < numRead; i++) {
        if (buffer[i] == fsbHeaderString[fsbStrIndex]) {
            assert(0 <= fsbStrIndex && fsbStrIndex <= 3);
            fsbStrIndex++;
            //if matched (ignore \0 character in fsbHeaderString)
            if (fsbStrIndex == 4) {
                fsbStrIndex = 0;
                if (resultCount >= resultArrLen) {
                    printf("LOG: More results were found than "
                           "what result array can hold.");
                    return resultCount;
                }
                resultArr[resultCount] = buffer[i];
                resultCount++;
            }
        }
    }
    return resultCount;
}

struct FSB readFile(const char* fileName) {
    struct FSB fsb = {};
    FILE *const fileHandle = fopen(fileName, "rb");

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

