#include "pcssb.hpp"

#include <iostream>
#include <filesystem>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "myIO.hpp"

std::size_t findFSBHeaderIndexes(
    const char *const inputFileName,
    std::size_t *const resultArr,
    const std::size_t resultArrLen) {
    assert(inputFileName != nullptr);
    assert(resultArr != nullptr);
    assert(resultArrLen > 0);

    //next index in resultArr to fill (0 indexed)
    //also happens to be the number of results currently found
    std::size_t resultCount { 0 };

    std::FILE *const fileHandle { myfopen(inputFileName, "rb") };
    {
        //how many buffers into the file
        std::size_t readIndex { 0 };

        while (!std::feof(fileHandle)) {
            constexpr std::size_t BUFFER_SIZE { 100 };
            std::uint32_t buffer[BUFFER_SIZE] {};
            static_assert((sizeof(buffer) / sizeof(std::uint32_t)) == BUFFER_SIZE);

            const std::size_t numRead = myfread(
                buffer,
                sizeof(std::uint32_t),
                BUFFER_SIZE,
                fileHandle);

            for (std::size_t i = 0; i < numRead; i++) {
                if (buffer[i] == FSB_HEADER_VALUE) {
                    if (resultCount >= resultArrLen) {
                        std::cout << "LOG: More results were found than "
                               "what result array can hold.\n";

                        (void) std::fclose(fileHandle);

                        return resultCount;
                    }
                    //position (in terms of how many longs into the file it is)
                    const size_t uint32Pos { i + (readIndex * BUFFER_SIZE) };
                    //get how many bytes into the file it is
                    resultArr[resultCount] = uint32Pos * sizeof(uint32_t);
                    resultCount++;
                }
            }
            readIndex++;
        }
    }
    (void) std::fclose(fileHandle);

    return resultCount;
}

void printFSBHeaderIndexes(const char *const fileName) {
    assert(fileName != nullptr);

    constexpr int BUFFER_SIZE { 100 };
    std::size_t fsbHeaderIndexes[BUFFER_SIZE] {};
    const std::size_t numResults { findFSBHeaderIndexes(fileName, fsbHeaderIndexes, BUFFER_SIZE) };
    for (std::size_t i = 0; i < numResults; i++) {
        (void) std::printf("%lu: decimal = %lu, hex = 0x%lX \n",
            i+1,
            fsbHeaderIndexes[i],
            fsbHeaderIndexes[i]
        );
    }
}

std::uint32_t readDataSize(
    const char *const inputFileName,
    const std::size_t fsb3HeaderPosition) {
    assert(inputFileName != nullptr);

    std::uint32_t dataSize { 0 };

    std::FILE *const fileHandle { myfopen(inputFileName, "rb") };
    {
        //set the file position indicator to start of FSB file
        myfseek_unsigned(fileHandle, fsb3HeaderPosition, SEEK_SET);
        //move to location where data size is written
        myfseek(fileHandle, DATA_SIZE_OFFSET, SEEK_CUR);

        //read data size long
        (void) myfread(&dataSize, sizeof(uint32_t), 1, fileHandle);
    }
    (void) std::fclose(fileHandle);

    return dataSize;
}

void readFileName(
    const char *const inputFileName,
    const std::size_t fsb3HeaderPosition,
    char resultArr[FSB_FILENAME_SIZE]) {
    assert(inputFileName != nullptr);
    assert(resultArr != nullptr);

    std::FILE *const fileHandle { myfopen(inputFileName, "rb") };
    {
        //set the file position indicator to start of FSB file
        myfseek_unsigned(fileHandle, fsb3HeaderPosition, SEEK_SET);
        //move to location where file name is written
        myfseek(fileHandle, FILENAME_OFFSET, SEEK_CUR);

        //read file name
        (void) myfread(resultArr, sizeof(char), FSB_FILENAME_SIZE, fileHandle);
    }
    (void) std::fclose(fileHandle);
    resultArr[FSB_FILENAME_SIZE - 1] = '\0';
}

void outputAudioData(
    const char *const inputFileName,
    const std::size_t fsb3HeaderPosition,
    const std::size_t headerSize,
    const std::size_t dataSize,
    const char *const outputFileName) {
    assert(inputFileName != nullptr);
    assert(outputFileName != nullptr);

    char *const audioData { static_cast<char *>(std::malloc(dataSize * sizeof(char))) };
    {
        std::FILE *const inputFileHandle { myfopen(inputFileName, "rb") };
        {
            //set the file position indicator to start of FSB file
            myfseek_unsigned(inputFileHandle, fsb3HeaderPosition, SEEK_SET);
            //move to start of audio data
            myfseek_unsigned(inputFileHandle, headerSize, SEEK_CUR);

            //read audio data
            (void) myfread(audioData, sizeof(char), dataSize, inputFileHandle);
        }
        (void) std::fclose(inputFileHandle);

        //write it to the output file
        std::FILE *const outputFileHandle { myfopen(outputFileName, "wb") };
        {
            (void) myfwrite(audioData, sizeof(char), dataSize, outputFileHandle);
        }
        (void) std::fclose(outputFileHandle);
    }
    std::free(audioData);
}

void outputAudioFiles(const char *const inputFileName) {
    assert(inputFileName != nullptr);

    constexpr int BUFFER_SIZE { 100 };
    std::size_t fsbIndexes[BUFFER_SIZE] {};
    const std::size_t numResults { findFSBHeaderIndexes(inputFileName, fsbIndexes, BUFFER_SIZE) };

    //we only look at the alternate found FSBs
    //(1st, 3rd) etc. because each one is duplicated in the PCSSB archive.
    //the duplicate doesn't have all of the data, so isn't worth outputting
    for (std::size_t i = 0; i < numResults; i += 2) {
        const std::uint32_t fsbDataSize { readDataSize(inputFileName, fsbIndexes[i]) };
        if (i < numResults - 1) {
            //apart from the last FSB, actual data size is just distance from the data start until the next FSB
            const std::size_t actualDataSize { fsbIndexes[i+1] - (fsbIndexes[i] + FSB_HEADER_SIZE) };
            if (fsbDataSize != actualDataSize) {
                std::cout << "LOG: Data size value doesn't match actual size!\n";
            }
            char fsbFileName[FSB_FILENAME_SIZE] {};
            readFileName(inputFileName, fsbIndexes[i], fsbFileName);

            //get location of file extension start
            /*TODO: should this be an assert? depends if this will be called from sm3tools.c
                which already has file extension validation*/
            const char* const fileExtensionPtr { std::strrchr(inputFileName, '.') };
            if (fileExtensionPtr == nullptr) {
                std::cerr << "ERROR: Input file doesn't have a file extension!\n";
                std::exit(EXIT_FAILURE);
            }
            const std::ptrdiff_t fileExtensionIndex { fileExtensionPtr - inputFileName };
            assert(fileExtensionIndex > 0);

            constexpr int OUTPUT_DIR_SIZE { 200 };
            char outputDir[OUTPUT_DIR_SIZE] {};
            //copy the head of the string until (excluding) the last '.'
            (void) std::snprintf(
                outputDir,
                static_cast<std::size_t>(fileExtensionIndex + 1),
                "%s",
                inputFileName);
            //create the directory corresponding to that path
            mymkdir(outputDir);

            //create path with file fsbFileName inside the output directory
            constexpr int OUTPUT_PATH_SIZE { OUTPUT_DIR_SIZE + 100 };
            char outputPath[OUTPUT_PATH_SIZE] {};
            (void) std::snprintf(
                outputPath,
                OUTPUT_PATH_SIZE,
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

std::size_t findFirstFSBMatchingFileName(
    const char *const pcssbFileName,
    const char *const fileNameString) {
    assert(pcssbFileName != nullptr);
    assert(fileNameString != nullptr);

    constexpr int BUFFER_SIZE { 100 };
    std::size_t fsbIndexes[BUFFER_SIZE] {};
    const std::size_t numResults { findFSBHeaderIndexes(pcssbFileName, fsbIndexes, BUFFER_SIZE) };
    for (std::size_t i = 0; i < numResults; i++) {
        char fsbFileName[FSB_FILENAME_SIZE] {};
        readFileName(pcssbFileName, fsbIndexes[i], fsbFileName);

        if (std::strcmp(fsbFileName, fileNameString) == 0) {
            return fsbIndexes[i];
        }
    }

    std::cerr << "ERROR: File not found in PCSSB!\n";
    std::exit(EXIT_FAILURE);
}

void readAndWriteToNewFile(
    const char *const inputFileName,
    const char *const outputFileName,
    const size_t readCount,
    const size_t readPosition,
    const bool append) {
    assert(inputFileName != nullptr);
    assert(outputFileName != nullptr);
    assert(readCount > 0);

    //store bytes from input in intermediate buffer
    //(plus one extra byte for null terminator)
    char *const buffer { static_cast<char *>(std::malloc((readCount + 1) * sizeof(char))) };
    {
        std::FILE *const inputFileHandle { myfopen(inputFileName, "rb") };
        {
            myfseek_unsigned(inputFileHandle, readPosition, SEEK_SET);

            const std::size_t numRead { myfread(buffer, sizeof(char), readCount, inputFileHandle) };
            //null terminate buffer string for safety
            buffer[numRead] = '\0';

            //either append or write depending on append argument
            //NOTE: we create an outputMode variable this way so that
            //we can keep outputFileHandle const
            const char *const outputMode { append ? "ab" : "wb" };
            std::FILE *const outputFileHandle { myfopen(outputFileName, outputMode) };
            {
                (void) myfwrite(buffer, sizeof(char), numRead, outputFileHandle);
            }
            (void) std::fclose(outputFileHandle);
        }
        (void) std::fclose(inputFileHandle);
    }
    std::free(buffer);
}

void replaceLongInFile(
    const char *const fileName,
    const std::size_t longPosition,
    const std::uint32_t newValue) {

    std::FILE *const fileHandle { myfopen(fileName, "r+b") };
    {
        //move to long position
        myfseek_unsigned(fileHandle, longPosition, SEEK_SET);

        //replace long
        const std::size_t numWritten { myfwrite(
            &newValue,
            sizeof(std::uint32_t),
            1,
            fileHandle) };

        if (numWritten != 1) {
            (void) std::fprintf(stderr, "ERROR: Error replacing long"
                            " at position %lu in %s!\n", longPosition, fileName);
            (void) std::fclose(fileHandle);
            std::exit(EXIT_FAILURE);
        }
    }
    (void) std::fclose(fileHandle);
}

void replaceAudioinPCSSB(
    const char *const pcssbFilePath,
    const char *const replaceFilePath) {

    //length of output path including null terminator
    //- byte for null terminator is included in sizeof("-mod")
    const std::size_t outputFilePathSize { (std::strlen(pcssbFilePath) + sizeof("-mod"))
        * sizeof(char) };
    char *const outputFilePath { static_cast<char *>(std::malloc(outputFilePathSize)) };
    {
        //generate output file name
        (void) std::snprintf(outputFilePath, outputFilePathSize, "%s-mod", pcssbFilePath);

        //find audio file in PCSSB using its filename (including file extension but excluding path)
        const std::filesystem::path audioFileName { std::filesystem::path{replaceFilePath}.filename() };

        const std::size_t fsbHeaderIndex = findFirstFSBMatchingFileName(pcssbFilePath, audioFileName.c_str());
        const std::uint32_t originalDataSize = readDataSize(pcssbFilePath, fsbHeaderIndex);
        const std::intmax_t replaceDataSize = getfilesize(replaceFilePath);
        const std::size_t fsbAudioDataIndex = fsbHeaderIndex + FSB_HEADER_SIZE;
        //append everything up to the existing audio data into the output file
        readAndWriteToNewFile(
            pcssbFilePath,
            outputFilePath,
            fsbAudioDataIndex,
            0,
            false);

        //append the replacement audio data into the output file
        readAndWriteToNewFile(
            replaceFilePath,
            outputFilePath,
            //NOTE: case where the data size is negative is logged in myIO.c.
            //apparently negative values for it are supposed to wrap around
            //to positive ones anyway so this should work. but it is not guaranteed to.
            static_cast<std::size_t>(replaceDataSize),
            0,
            true);

        //append the rest of the original file after the audio data
        readAndWriteToNewFile(
            pcssbFilePath,
            outputFilePath,
            //NOTE: this will overflow but it shouldn't matter
            //ensures all the bytes after from original file is read
            static_cast<std::size_t>(getfilesize(pcssbFilePath)),
            fsbAudioDataIndex + originalDataSize,
            true);

        //NOTE: this warning is unlikely to be reachable on most platforms
        //but is just there for portability. The data size can't be more than 4 bits
        if (replaceDataSize > UINT32_MAX) {
            std::cerr << "WARNING: Replacement audio data size is too large"
                            ", replacement may not work properly!\n";
        }
        //change data size field to match size of replaceFilePath
        replaceLongInFile(
            outputFilePath,
            (fsbHeaderIndex + DATA_SIZE_OFFSET),
            static_cast<std::uint32_t>(replaceDataSize));
    }
    std::free(outputFilePath);
}

int main(const int argc, const char *const argv[]) {
    if (argc < 2) {
        std::cerr << "ERROR: This program needs "
                               "at least one input argument.";
        return EXIT_FAILURE;
    }
    if (argc == 2) {
        (void) std::printf("INFO: Extracting audio from %s\n", argv[1]);
        //printFSBHeaderIndexes(argv[1]);
        outputAudioFiles(argv[1]);
    }
    if (argc > 3) {
        std::cerr << "WARNING: Arguments after the 2nd"
                        " argument are currently ignored.\n";
    }
    if (argc >= 3) {
        (void) std::printf("Replacing %s in %s\n", argv[2], argv[1]);
        replaceAudioinPCSSB(argv[1], argv[2]);
    }
    return EXIT_SUCCESS;
}

