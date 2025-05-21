#include "pcssb.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

//finds every instance of "FSB3" header text in the sound file
//puts the absolute position in bytes of those instances
//into the given resultArr in the order they are found.
//return value is number of results found. if it returns a value greater than resultArrLen,
//that means resultArr was too small and there may have been more results that couldn't fit.
//NOTE: we assume that the file is aligned in terms of longs (i.e. position of header in file
//is divisible by 4). In the case of Spider-Man 3 PCSSB files this seems to be the case.
size_t findFSBHeaderIndexes(
    const char *const inputFileName,
    size_t *const resultArr,
    const size_t resultArrLen) {
    //next index in resultArr to fill (0 indexed)
    //also happens to be the number of results currently found
    size_t resultCount = 0;

    FILE *const fileHandle = fopen(inputFileName, "rb");
    if (!fileHandle) {
        perror("ERROR: Failed to open file");
        exit(EXIT_FAILURE);
    }

    //how many buffers into the file
    size_t readIndex = 0;

    while (!feof(fileHandle)) {
        const size_t buffSize = 100;
        //magic number needs to be reused to avoid Variable Length Array
        uint32_t buffer[100] = {0};
        assert((sizeof(buffer) / sizeof(uint32_t)) == buffSize);

        const size_t numRead = fread(buffer, sizeof(uint32_t), buffSize , fileHandle);
        if (ferror(fileHandle)) {
            perror("ERROR: I/O error when reading");
        }
        assert(numRead <= buffSize);
        // if (numRead < buffSize) {
        //     (void) printf("LOG: buffSize is %lu, amount read was only %lu.\n", buffSize, numRead);
        // }

        for (size_t i = 0; i < numRead; i++) {
            //string to match in the file, represented as an unsigned long
            const uint32_t fsbHeader = 859984710; // = "FSB3"
            if (buffer[i] == fsbHeader) {
                if (resultCount >= resultArrLen) {
                    (void) printf("LOG: More results were found than "
                           "what result array can hold.");

                    fclose(fileHandle);

                    return resultCount;
                }
                //position (in terms of how many longs into the file it is)
                const size_t uint32Pos = i + (readIndex * buffSize);
                //get how many bytes into the file it is
                resultArr[resultCount] = uint32Pos * sizeof(uint32_t);
                resultCount++;
            }
        }

        readIndex++;
    }

    (void) fclose(fileHandle);

    return resultCount;
}

struct FSB readFile(const char* fileName) {
    struct FSB fsb = {0};
    FILE *const fileHandle = fopen(fileName, "rb");
    if (!fileHandle) {
        perror("ERROR: Failed to open file");
        exit(EXIT_FAILURE);
    }

    const size_t count = 100;
    uint32_t buffer[100] = {0};

    const size_t numRead = fread(buffer, 4, count , fileHandle);
    if (ferror(fileHandle)) {
        perror("ERROR: I/O error when reading");
    }
    if (numRead < count) {
        (void) printf("LOG: count was %lu, amount read was only %lu.\n", count, numRead);
    }

    fsb.fsb3Header = buffer[0];
    fsb.numFiles = buffer[1];
    fsb.unknown1 = buffer[2];
    fsb.dataSize = buffer[3];
    fsb.unknown2 = buffer[4];
    fsb.null1 = buffer[5];

    (void) fclose(fileHandle);

    return fsb;
}

int main(int argc, char* argv[]) {
    size_t* result = (size_t*) malloc(sizeof(size_t) * 100);
    if (result == NULL) {
        (void) fprintf(stderr, "ERROR: Failed to malloc result.\n");
        exit(EXIT_FAILURE);
    }
    size_t numResults = findFSBHeaderIndexes(argv[1], result, 100);
    for (size_t i = 0; i < numResults; i++) {
        (void) printf("%lu: decimal=%lu, hex = 0x%lX \n",
            i,
            result[i],
            result[i]
        );
    }
    free(result);
}

