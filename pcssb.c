#include "pcssb.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "myIO.h"

//number of bytes used to store the sample filename in FSB archives,
//EXCLUDING the "P\0" at the start for simplicity
#define FSB_FILENAME_SIZE 30

struct FSB readFile(const char* fileName) {
    struct FSB fsb = {0};

    const size_t BUFFER_SIZE = 100;
    uint32_t buffer[100] = {0};

    FILE *const fileHandle = myfopen(fileName, "rb");
    (void) myfread(buffer, sizeof(uint32_t), BUFFER_SIZE, fileHandle);
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

    const int DATA_SIZE_OFFSET = 3 * sizeof(uint32_t);

    //set the file position indicator to start of FSB file
    myfseek_unsigned(fileHandle, fsb3HeaderPosition, SEEK_SET);
    //move to location where data size is written
    myfseek(fileHandle, DATA_SIZE_OFFSET, SEEK_CUR);

    //read data size long
    uint32_t dataSize = 0;
    (void) myfread(&dataSize, sizeof(uint32_t), 1, fileHandle);

    (void) fclose(fileHandle);

    return dataSize;
}

void readFileName(
    const char *const inputFileName,
    const size_t fsb3HeaderPosition,
    char resultArr[FSB_FILENAME_SIZE]) {

    FILE *const fileHandle = myfopen(inputFileName, "rb");

    //NOTE: set 2 bytes ahead because otherwise
    //it seems to begin with (P\0) which would null terminate the string
    //which is annoying, and I'm not sure if that's supposed
    //to be part of the file name anyway or if it's something else.
    const int FILENAME_OFFSET = 2 + (6 * sizeof(uint32_t));

    //set the file position indicator to start of FSB file
    myfseek_unsigned(fileHandle, fsb3HeaderPosition, SEEK_SET);
    //move to location where file name is written
    myfseek(fileHandle, FILENAME_OFFSET, SEEK_CUR);

    //read file name
    (void) myfread(resultArr, sizeof(char), FSB_FILENAME_SIZE, fileHandle);

    (void) fclose(fileHandle);
}

//Outputs the audio data in folder structure matching input file name.
//e.g. for file "a.wav" in "b.PCSSB" the output will be in b/a.wav
void outputAudioData(
    const char *const inputFileName,
    const size_t fsb3HeaderPosition,
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
    (void) myfread(audioData, sizeof(char), dataSize, inputFileHandle);
    (void) fclose(inputFileHandle);

    //write it to the output file
    FILE *const outputFileHandle = myfopen(outputFileName, "wb");
    (void) myfwrite(audioData, sizeof(char), dataSize, outputFileHandle);
    (void) fclose(outputFileHandle);

    free(audioData);
}

//Outputs the audio data in folder structure matching input file name.
//e.g. for file "a.wav" in "b.PCSSB" the output will be in b/a.wav
void outputAudioFiles(const char *const inputFileName) {
    size_t fsbIndexes[100] = {0};
    const size_t numResults = findFSBHeaderIndexes(inputFileName, fsbIndexes, 100);

    const int FSB_HEADER_SIZE = 104;

    //we only look at the alternate found FSBs
    //(1st, 3rd) etc. because each one is duplicated in the PCSSB archive.
    //the duplicate doesn't have all of the data, so isn't worth outputting
    for (size_t i = 0; i < numResults; i++) {
        const uint32_t fsbDataSize = readDataSize(inputFileName, fsbIndexes[i]);
        if (i < numResults - 1) {
            //apart from the last FSB, actual data size is just distance from the data start until the next FSB
            const size_t actualDataSize = fsbIndexes[i+1] - (fsbIndexes[i] + FSB_HEADER_SIZE);
            if (fsbDataSize != actualDataSize) {
                (void) printf("LOG: Data size value doesn't match actual size!");
            }
            char fsbFileName[FSB_FILENAME_SIZE] = {0};
            readFileName(inputFileName, fsbIndexes[i], fsbFileName);
            char outputFileName[200] = {0};
            (void) snprintf(outputFileName, 200, "%s-%lu-%s", inputFileName, i, fsbFileName);
            outputAudioData(
                inputFileName,
                fsbIndexes[i],
                FSB_HEADER_SIZE,
                actualDataSize,
                outputFileName);
        }
    }
}

int main(const int argc, const char *const argv[]) {
    //printFSBHeaderIndexes(argv[1]);
    outputAudioFiles(argv[1]);
}

