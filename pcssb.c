#include "pcssb.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "myIO.h"

struct FSB readFile(const char* fileName) {
    struct FSB fsb = {0};

    const size_t BUFFER_SIZE = 100;
    uint32_t buffer[100] = {0};

    FILE *const fileHandle = myfopen(fileName, "rb");
    (void) myfread(buffer, 4, BUFFER_SIZE, fileHandle);
    (void) fclose(fileHandle);

    fsb.fsb3Header = buffer[0];
    fsb.numFiles = buffer[1];
    fsb.unknown1 = buffer[2];
    fsb.dataSize = buffer[3];
    fsb.unknown2 = buffer[4];
    fsb.null1 = buffer[5];

    return fsb;
}

size_t findFSBHeaderIndexes(
    const char *const inputFileName,
    size_t *const resultArr,
    const size_t resultArrLen) {

    if (resultArr == NULL) {
        (void) fprintf(stderr, "ERROR: Result Array is NULL!.\n");
        exit(EXIT_FAILURE);
    }

    //next index in resultArr to fill (0 indexed)
    //also happens to be the number of results currently found
    size_t resultCount = 0;

    FILE *const fileHandle = myfopen(inputFileName, "rb");

    //how many buffers into the file
    size_t readIndex = 0;

    while (!feof(fileHandle)) {
        const size_t BUFFER_SIZE = 100;
        //magic number needs to be reused to avoid Variable Length Array
        uint32_t buffer[100] = {0};
        assert((sizeof(buffer) / sizeof(uint32_t)) == BUFFER_SIZE);

        const size_t numRead = myfread(buffer, sizeof(uint32_t), BUFFER_SIZE, fileHandle);

        for (size_t i = 0; i < numRead; i++) {
            //string to match in the file, represented as an unsigned long
            const uint32_t FSB_HEADER = 859984710; // = "FSB3"
            if (buffer[i] == FSB_HEADER) {
                if (resultCount >= resultArrLen) {
                    (void) printf("LOG: More results were found than "
                           "what result array can hold.\n");

                    (void) fclose(fileHandle);

                    return resultCount;
                }
                //position (in terms of how many longs into the file it is)
                const size_t uint32Pos = i + (readIndex * BUFFER_SIZE);
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

void printFSBHeaderIndexes(const char *const fileName) {
    size_t fsbHeaderIndexes[100] = {0};
    const size_t numResults = findFSBHeaderIndexes(fileName, fsbHeaderIndexes, 100);
    for (size_t i = 0; i < numResults; i++) {
        (void) printf("%lu: decimal = %lu, hex = 0x%lX \n",
            i+1,
            fsbHeaderIndexes[i],
            fsbHeaderIndexes[i]
        );
    }
}

uint32_t readDataSize(
    const char *const inputFileName,
    const size_t fsb3HeaderPosition) {

    FILE *const fileHandle = myfopen(inputFileName, "rb");

    //set the file position indicator to start of FSB file
    myfseek_unsigned(fileHandle, fsb3HeaderPosition, SEEK_SET);

    //move to location where data size is written
    myfseek(fileHandle, 3 * sizeof(uint32_t), SEEK_CUR);

    //read data size long
    uint32_t dataSize = 0;
    (void) myfread(&dataSize, sizeof(uint32_t), 1, fileHandle);

    (void) fclose(fileHandle);

    return dataSize;
}

//Outputs the audio data in folder structure matching input file name.
//e.g. for file "a.wav" in "b.PCSSB" the output will be in b/a.wav
void outputAudioData(
    const char *const inputFileName,
    const size_t fsb3HeaderPosition,
    //TODO should we use size_t for this?
    const size_t headerSize,
    const size_t dataSize,
    const char *const outputFileName) {

    FILE *const inputFileHandle = myfopen(inputFileName, "rb");

    //set the file position indicator to start of FSB file
    myfseek_unsigned(inputFileHandle, fsb3HeaderPosition, SEEK_SET);
    //move to start of audio data
    myfseek_unsigned(inputFileHandle, headerSize, SEEK_CUR);

    //read audio data
    char *const audioData = malloc(dataSize * sizeof(char));
    (void) myfread(audioData, 1, dataSize, inputFileHandle);
    (void) fclose(inputFileHandle);

    //write it to the output file
    FILE *const outputFileHandle = myfopen(outputFileName, "wb");
    (void) myfwrite(audioData, 1, dataSize, outputFileHandle);
    (void) fclose(outputFileHandle);

    free(audioData);
}

//Outputs the audio data in folder structure matching input file name.
//e.g. for file "a.wav" in "b.PCSSB" the output will be in b/a.wav
void outputAudioFiles(const char *const inputFileName) {
    size_t fsbIndexes[100] = {0};
    const size_t numResults = findFSBHeaderIndexes(inputFileName, fsbIndexes, 100);

    const int HEADER_SIZE = 104;

    //we only look at the alternate found FSBs
    //(1st, 3rd) etc. because each one is duplicated in the PCSSB archive.
    //the duplicate doesn't have all of the data, so isn't worth outputting
    for (size_t i = 0; i < numResults; i += 2) {
        const uint32_t dataSize = readDataSize(inputFileName, fsbIndexes[i]);
        //apart from the last FSB, actual data size iis
        if (i < numResults - 1) {
            if (dataSize != (fsbIndexes[i+1] - (fsbIndexes[i] + HEADER_SIZE))) {
                (void) printf("LOG: Data size value doesn't match actual size!");
            }
            char outputFileName[100] = {0};
            (void) snprintf(outputFileName, 100, "%s-output%lu.wav", inputFileName, i/2);
            outputAudioData(inputFileName, fsbIndexes[i], HEADER_SIZE, dataSize, outputFileName);
        }
    }
}

int main(const int argc, const char *const argv[]) {
    outputAudioFiles(argv[1]);
}

