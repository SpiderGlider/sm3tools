#include "pcssb.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "myIO.h"

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

void outputAudioFiles(const char *const inputFileName) {
    size_t fsbIndexes[100] = {0};
    const size_t numResults = findFSBHeaderIndexes(inputFileName, fsbIndexes, 100);

    //we only look at the alternate found FSBs
    //(1st, 3rd) etc. because each one is duplicated in the PCSSB archive.
    //the duplicate doesn't have all of the data, so isn't worth outputting
    for (size_t i = 0; i < numResults; i += 2) {
        const uint32_t fsbDataSize = readDataSize(inputFileName, fsbIndexes[i]);
        if (i < numResults - 1) {
            //apart from the last FSB, actual data size is just distance from the data start until the next FSB
            const size_t actualDataSize = fsbIndexes[i+1] - (fsbIndexes[i] + FSB_HEADER_SIZE);
            if (fsbDataSize != actualDataSize) {
                (void) printf("LOG: Data size value doesn't match actual size!");
            }
            char fsbFileName[FSB_FILENAME_SIZE] = {0};
            readFileName(inputFileName, fsbIndexes[i], fsbFileName);

            //get location of file extension start
            const size_t fileExtensionIndex = strrchr(inputFileName, '.') - inputFileName;
            char outputDir[200] = {0};
            //copy the head of the string until and including the last '.'
            (void) strncpy(outputDir, inputFileName, fileExtensionIndex + 1);
            //replace '.' with null terminator
            outputDir[fileExtensionIndex] = '\0';
            //create the directory corresponding to that path
            mymkdir(outputDir);

            //create path with file fsbFileName inside the output directory
            char outputPath[300] = {0};
            (void) snprintf(outputPath, 300, "%s/%s", outputDir, fsbFileName);
            outputAudioData(
                inputFileName,
                fsbIndexes[i],
                FSB_HEADER_SIZE,
                fsbDataSize,
                outputPath);
        }
    }
}

size_t findFirstFSBMatchingFileName(
    const char *const pcssbFileName,
    const char *const fileNameString) {

    size_t fsbIndexes[100] = {0};
    const size_t numResults = findFSBHeaderIndexes(pcssbFileName, fsbIndexes, 100);
    for (size_t i = 0; i < numResults; i++) {
        char fsbFileName[FSB_FILENAME_SIZE] = {0};
        readFileName(pcssbFileName, fsbIndexes[i], fsbFileName);

        if (strcmp(fsbFileName, fileNameString) == 0) {
            return fsbIndexes[i];
        }
    }

    fprintf(stderr, "ERROR: File not found in PCSSB!\n");
    exit(EXIT_FAILURE);
}

void replaceAudioinPCSSB(
    const char *const pcssbFileName,
    const char *const replaceFileName) {

    //find filename in PCSSB file
    size_t fsbHeaderIndex = findFirstFSBMatchingFileName(pcssbFileName, replaceFileName);


    FILE *const pcssbFileHandle = myfopen(pcssbFileName, "rb");
    //store all bytes up until start of audio data
    //(plus one extra byte for the null terminator)
    const size_t fsbAudioDataIndex = fsbHeaderIndex + FSB_HEADER_SIZE;
    char *const pcssbHead = (char*) malloc((fsbAudioDataIndex + 1) * sizeof(char));
    myfread(pcssbHead, sizeof(char), fsbAudioDataIndex, pcssbFileHandle);
    //null terminate string for safety reasons
    pcssbHead[fsbAudioDataIndex] = '\0';

    //write into new file
    //TODO use an output filename based on the pcssb file name with "-mod" at the end
    FILE *const outputFileHandle = myfopen("test-out.pcssb", "wb");
    myfwrite(pcssbHead, sizeof(char), fsbAudioDataIndex, outputFileHandle);

    const intmax_t replaceSize = getfilesize(replaceFileName);
    char *const replaceData = (char*) malloc((fsbAudioDataIndex + 1) * sizeof(char));
    free(replaceData);

    fclose(outputFileHandle);

    free(pcssbHead);

    fclose(pcssbFileHandle);


    //TODO go to audio data part of that fsb

    //TODO write all the bytes from the file up till that point into a new file

    //TODO append all the data in replaceFileName at the end of that new file

    //TODO append all the bytes after the audio data end from the original into the new file

    //TODO change data size field to match size of replaceFileName
}

int main(const int argc, const char *const argv[]) {
    //printFSBHeaderIndexes(argv[1]);
    outputAudioFiles(argv[1]);
}

