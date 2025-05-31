#include "pcssb.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "myIO.hpp"

size_t findFSBHeaderIndexes(
    const char *const inputFileName,
    size_t *const resultArr,
    const size_t resultArrLen) {

    if (resultArr == nullptr) {
        (void) fprintf(stderr, "ERROR: Result Array is NULL!.\n");
        std::exit(EXIT_FAILURE);
    }

    //next index in resultArr to fill (0 indexed)
    //also happens to be the number of results currently found
    size_t resultCount = 0;

    FILE *const fileHandle = myfopen(inputFileName, "rb");
    {
        //how many buffers into the file
        size_t readIndex = 0;

        while (!feof(fileHandle)) {
            constexpr size_t BUFFER_SIZE = 100;
            //magic number needs to be reused to avoid Variable Length Array
            uint32_t buffer[100] = {0};
            assert((sizeof(buffer) / sizeof(uint32_t)) == BUFFER_SIZE);

            const size_t numRead = myfread(buffer, sizeof(uint32_t), BUFFER_SIZE, fileHandle);

            for (size_t i = 0; i < numRead; i++) {
                //string to match in the file, represented as an unsigned long
                constexpr uint32_t FSB_HEADER = 859984710; // = "FSB3"
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

    uint32_t dataSize = 0;

    constexpr int DATA_SIZE_OFFSET = 3 * sizeof(uint32_t);

    FILE *const fileHandle = myfopen(inputFileName, "rb");
    {
        //set the file position indicator to start of FSB file
        myfseek_unsigned(fileHandle, fsb3HeaderPosition, SEEK_SET);
        //move to location where data size is written
        myfseek(fileHandle, DATA_SIZE_OFFSET, SEEK_CUR);

        //read data size long
        (void) myfread(&dataSize, sizeof(uint32_t), 1, fileHandle);
    }
    (void) fclose(fileHandle);

    return dataSize;
}

void readFileName(
    const char *const inputFileName,
    const size_t fsb3HeaderPosition,
    char resultArr[FSB_FILENAME_SIZE]) {

    FILE *const fileHandle = myfopen(inputFileName, "rb");
    {
        //NOTE: set 2 bytes ahead because otherwise
        //it seems to begin with (P\0) which would null terminate the string
        //which is annoying, and I'm not sure if that's supposed
        //to be part of the file name anyway or if it's something else.
        constexpr int FILENAME_OFFSET = 2 + (6 * sizeof(uint32_t));

        //set the file position indicator to start of FSB file
        myfseek_unsigned(fileHandle, fsb3HeaderPosition, SEEK_SET);
        //move to location where file name is written
        myfseek(fileHandle, FILENAME_OFFSET, SEEK_CUR);

        //read file name
        (void) myfread(resultArr, sizeof(char), FSB_FILENAME_SIZE, fileHandle);
    }
    (void) fclose(fileHandle);
    resultArr[FSB_FILENAME_SIZE - 1] = '\0';
}

void outputAudioData(
    const char *const inputFileName,
    const size_t fsb3HeaderPosition,
    const size_t headerSize,
    const size_t dataSize,
    const char *const outputFileName) {

    char *const audioData = static_cast<char *>(malloc(dataSize * sizeof(char)));
    {
        FILE *const inputFileHandle = myfopen(inputFileName, "rb");
        {
            //set the file position indicator to start of FSB file
            myfseek_unsigned(inputFileHandle, fsb3HeaderPosition, SEEK_SET);
            //move to start of audio data
            myfseek_unsigned(inputFileHandle, headerSize, SEEK_CUR);

            //read audio data
            (void) myfread(audioData, sizeof(char), dataSize, inputFileHandle);
        }
        (void) fclose(inputFileHandle);

        //write it to the output file
        FILE *const outputFileHandle = myfopen(outputFileName, "wb");
        {
            (void) myfwrite(audioData, sizeof(char), dataSize, outputFileHandle);
        }
        (void) fclose(outputFileHandle);
    }
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
                (void) printf("LOG: Data size value doesn't match actual size!\n");
            }
            char fsbFileName[FSB_FILENAME_SIZE] = {0};
            readFileName(inputFileName, fsbIndexes[i], fsbFileName);

            //get location of file extension start
            /*TODO: should this be an assert? depends if this will be called from sm3tools.c
                which already has file extension validation*/
            const char* const fileExtensionPtr = strrchr(inputFileName, '.');
            if (fileExtensionPtr == nullptr) {
                fprintf(stderr, "ERROR: Input file doesn't have a file extension!\n");
                std::exit(EXIT_FAILURE);
            }
            const ptrdiff_t fileExtensionIndex = fileExtensionPtr - inputFileName;
            assert(fileExtensionIndex > 0);

            char outputDir[200] = {0};
            //copy the head of the string until (excluding) the last '.'
            (void) snprintf(
                outputDir,
                static_cast<size_t>(fileExtensionIndex + 1),
                "%s",
                inputFileName);
            //create the directory corresponding to that path
            mymkdir(outputDir);

            //create path with file fsbFileName inside the output directory
            char outputPath[300] = {0};
            (void) snprintf(
                outputPath,
                300,
                "%s/%s",
                outputDir,
                fsbFileName);
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
    std::exit(EXIT_FAILURE);
}

void readAndWriteToNewFile(
    const char *const inputFileName,
    const char *const outputFileName,
    const size_t readCount,
    const size_t readPosition,
    const bool append) {

    assert(append == false || append == true);

    //store bytes from input in intermediate buffer
    //(plus one extra byte for null terminator)
    char *const buffer = static_cast<char *>(malloc((readCount + 1) * sizeof(char)));
    {
        FILE *const inputFileHandle = myfopen(inputFileName, "rb");
        {
            myfseek_unsigned(inputFileHandle, readPosition, SEEK_SET);

            const size_t numRead = myfread(buffer, sizeof(char), readCount, inputFileHandle);
            //null terminate buffer string for safety
            buffer[numRead] = '\0';

            //either append or write depending on append argument
            //NOTE: we create an outputMode variable this way so that
            //we can keep outputFileHandle const
            const char *const outputMode = append ? "ab" : "wb";
            FILE *const outputFileHandle = myfopen(outputFileName, outputMode);
            {
                (void) myfwrite(buffer, sizeof(char), numRead, outputFileHandle);
            }
            (void) fclose(outputFileHandle);
        }
        (void) fclose(inputFileHandle);
    }
    free(buffer);
}

void replaceLongInFile(
    const char *const fileName,
    const size_t longPosition,
    const uint32_t newValue) {

    FILE *const fileHandle = myfopen(fileName, "r+b");
    {
        //move to long position
        myfseek_unsigned(fileHandle, longPosition, SEEK_SET);

        //replace long
        const size_t numWritten = myfwrite(
            &newValue,
            sizeof(uint32_t),
            1,
            fileHandle);

        if (numWritten != 1) {
            (void) fprintf(stderr, "ERROR: Error replacing long"
                            " at position %lu in %s!\n", longPosition, fileName);
            (void) fclose(fileHandle);
            std::exit(EXIT_FAILURE);
        }
    }
    (void) fclose(fileHandle);
}

void replaceAudioinPCSSB(
    const char *const pcssbFileName,
    const char *const replaceFileName) {

    //length of output path including null terminator
    //- byte for null terminator is included in sizeof("-mod")
    const size_t outputFileNameSize = (strlen(pcssbFileName) + sizeof("-mod"))
        * sizeof(char);
    char *const outputFileName = static_cast<char *>(malloc(outputFileNameSize));
    {
        //generate output file name
        (void) snprintf(outputFileName, outputFileNameSize, "%s-mod", pcssbFileName);

        //find audio file in PCSSB
        const size_t fsbHeaderIndex = findFirstFSBMatchingFileName(pcssbFileName, replaceFileName);
        const uint32_t originalDataSize = readDataSize(pcssbFileName, fsbHeaderIndex);
        const intmax_t replaceDataSize = getfilesize(replaceFileName);
        const size_t fsbAudioDataIndex = fsbHeaderIndex + FSB_HEADER_SIZE;
        //append everything up to the existing audio data into the output file
        readAndWriteToNewFile(
            pcssbFileName,
            outputFileName,
            fsbAudioDataIndex,
            0,
            false);

        //append the replacement audio data into the output file
        readAndWriteToNewFile(
            replaceFileName,
            outputFileName,
            //NOTE: case where the data size is negative is logged in myIO.c.
            //apparently negative values for it are supposed to wrap around
            //to positive ones anyway so this should work. but it is not guaranteed to.
            static_cast<size_t>(replaceDataSize),
            0,
            true);

        //append the rest of the original file after the audio data
        readAndWriteToNewFile(
            pcssbFileName,
            outputFileName,
            //NOTE: this will overflow but it shouldn't matter
            //ensures all the bytes after from original file is read
            static_cast<size_t>(getfilesize(pcssbFileName)),
            fsbAudioDataIndex + originalDataSize,
            true);

        constexpr int DATA_SIZE_OFFSET = 3 * sizeof(uint32_t);

        //NOTE: this warning is unlikely to be reachable on most platforms
        //but is just there for portability. The data size can't be more than 4 bits
        if (replaceDataSize > UINT32_MAX) {
            (void) fprintf(stderr, "WARNING: Replacement audio data size is too large"
                            ", replacement may not work properly!\n");
        }
        //change data size field to match size of replaceFileName
        replaceLongInFile(
            outputFileName,
            (fsbHeaderIndex + DATA_SIZE_OFFSET),
            static_cast<uint32_t>(replaceDataSize));
    }
    free(outputFileName);
}

int main(const int argc, const char *const argv[]) {
    if (argc < 2) {
        (void) fprintf(stderr, "ERROR: This program needs "
                               "at least one input argument.");
        return EXIT_FAILURE;
    }
    if (argc == 2) {
        (void) printf("INFO: Extracting audio from %s\n", argv[1]);
        //printFSBHeaderIndexes(argv[1]);
        outputAudioFiles(argv[1]);
    }
    if (argc > 3) {
        (void) fprintf(stderr, "WARNING: Arguments after the 2nd"
                        " argument are currently ignored.\n");
    }
    if (argc >= 3) {
        (void) printf("Replacing %s in %s\n", argv[2], argv[1]);
        replaceAudioinPCSSB(argv[1], argv[2]);
    }
    return EXIT_SUCCESS;
}

