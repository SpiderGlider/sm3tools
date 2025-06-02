#include "pcssb.hpp"

#include <iostream>
#include <filesystem>
#include <algorithm>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "myIO.hpp"

std::vector<size_t> findFSBIndexes(const char *const filePath) {
    assert(filePath != nullptr);

    std::vector<size_t> fsbIndexes {};
    //PCSSBs can have a lot more FSBs than this (though seldom more than 50),
    //but this expands in a way that should minimise the number of reallocations
    fsbIndexes.reserve(12);

    const auto fileSize = static_cast<size_t>(getfilesize(filePath));
    char *const buffer = new char[fileSize];
    {
        //NOTE: we assume that result of getfilesize is the actual file size
        std::FILE *const fileHandle { myfopen(filePath, "rb") };
        {
            //read entire file into buffer
            const std::size_t numRead = myfread(
                buffer,
                sizeof(char),
                fileSize,
                fileHandle);
            assert(numRead == fileSize);

            //build a string view with length added in for safety
            const std::string_view bufferSV { buffer, numRead };
            assert(bufferSV.size() == fileSize);

            //search from start of the file
            size_t searchStartPos = 0;
            bool isFullySearched { false };
            while (!isFullySearched) {
                assert(searchStartPos < bufferSV.size());
                //look for next occurrence of "FSB3" substring
                const size_t fsbIndex = bufferSV.find(FSB_MAGIC_STRING, searchStartPos);
                assert(fsbIndex >= searchStartPos);
                //if an occurrence of the substring was found
                if (fsbIndex != std::string_view::npos) {
                    fsbIndexes.push_back(fsbIndex);
                    //move the start index for the next search to after the found occurrence
                    searchStartPos = fsbIndex + FSB_MAGIC_STRING.length();
                }
                else {
                    isFullySearched = true;
                }
            }
        }
        (void) std::fclose(fileHandle);
    }
    delete[] buffer;
    return fsbIndexes;
}

void printFSBHeaderIndexes(const char *const filePath) {
    assert(filePath != nullptr);

    const std::vector<size_t> indexes { findFSBIndexes(filePath) };
    for (std::size_t i = 0; i < indexes.size(); i += 2) {
        char buffer[FSB_FILENAME_SIZE];
        readFileName(filePath, indexes.at(i), buffer);
        if (i < indexes.size() - 2) {
            std::printf("%lu: "
                        "file name %s, "
                        "hex address = 0x%lX, "
                        "fsb size = %lu, "
                        "duplicate size = %lu, "
                        "total size including duplicate = %lu\n",
                        i+1,
                        buffer,
                        indexes.at(i),
                        indexes.at(i+1)-indexes.at(i),
                        indexes.at(i+2)-indexes.at(i+1),
                        indexes.at(i+2)-indexes.at(i));
        }
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

    const std::vector<std::size_t> fsbIndexes { findFSBIndexes(inputFileName) };

    //we only look at the alternate found FSBs
    //(1st, 3rd) etc. because each one is duplicated in the PCSSB archive.
    //the duplicate doesn't have all of the data, so isn't worth outputting
    for (std::size_t i = 0; i < fsbIndexes.size(); i += 2) {
        const std::uint32_t fsbDataSize { readDataSize(inputFileName, fsbIndexes[i]) };
        if (i < (fsbIndexes.size() - 1)) {
            //apart from the last FSB, actual data size is just distance from the data start until the next FSB
            const std::size_t actualDataSize { fsbIndexes[i+1] - (fsbIndexes[i] + FSB_HEADER_SIZE) };
            if (fsbDataSize != actualDataSize) {
                std::cout << "LOG: Data size value doesn't match actual size!\n";
            }
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

std::size_t findFirstFSBMatchingFileName(
    const char *const pcssbFileName,
    const char *const fileNameString) {
    assert(pcssbFileName != nullptr);
    assert(fileNameString != nullptr);

    const std::vector<std::size_t> fsbIndexes = findFSBIndexes(pcssbFileName);

    for (size_t fsbIndex : fsbIndexes) {
        char fsbFileName[FSB_FILENAME_SIZE] {};
        readFileName(pcssbFileName, fsbIndex, fsbFileName);

        if (std::strcmp(fsbFileName, fileNameString) == 0) {
            return fsbIndex;
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
    const bool append,
    const bool padWithZeroes) {
    assert(inputFileName != nullptr);
    assert(outputFileName != nullptr);
    assert(readCount > 0);

    //store bytes from input in intermediate buffer
    //(plus one extra byte for null terminator)
    char *const buffer { static_cast<char *>(std::calloc(readCount + 1, sizeof(char))) };
    {
        std::FILE *const inputFileHandle { myfopen(inputFileName, "rb") };
        {
            myfseek_unsigned(inputFileHandle, readPosition, SEEK_SET);

            const std::size_t numRead { myfread(buffer, sizeof(char), readCount, inputFileHandle) };

            const std::size_t numToWrite { padWithZeroes ? readCount : numRead };

            //null terminate buffer string for safety
            buffer[numToWrite] = '\0';

            //either append or write depending on append argument
            //NOTE: we create an outputMode variable this way so that
            //we can keep outputFileHandle const
            const char *const outputMode { append ? "ab" : "wb" };
            std::FILE *const outputFileHandle { myfopen(outputFileName, outputMode) };
            {
                (void) myfwrite(buffer, sizeof(char), numToWrite, outputFileHandle);
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
        //write everything up to the existing audio data into the output file
        readAndWriteToNewFile(
            pcssbFilePath,
            outputFilePath,
            fsbAudioDataIndex,
            0,
            false,
            false);

        //append the replacement audio data into the output file
        readAndWriteToNewFile(
            replaceFilePath,
            outputFilePath,
            //NOTE: case where the data size is negative is logged in myIO.c.
            //apparently negative values for it are supposed to wrap around
            //to positive ones anyway so this should work. but it is not guaranteed to.
            originalDataSize,
            0,
            true,
            true);

        //append the rest of the original file after the audio data
        readAndWriteToNewFile(
            pcssbFilePath,
            outputFilePath,
            //NOTE: this will overflow but it shouldn't matter
            //ensures all the bytes after from original file is read
            static_cast<std::size_t>(getfilesize(pcssbFilePath)),
            fsbAudioDataIndex + originalDataSize,
            true,
            false);

        // //NOTE: this warning is unlikely to be reachable on most platforms
        // //but is just there for portability. The data size can't be more than 4 bits
        // if (replaceDataSize > UINT32_MAX) {
        //     std::cerr << "WARNING: Replacement audio data size is too large"
        //                     ", replacement may not work properly!\n";
        // }
        // //change data size field to match size of replaceFilePath
        // replaceLongInFile(
        //     outputFilePath,
        //     (fsbHeaderIndex + DATA_SIZE_OFFSET),
        //     static_cast<std::uint32_t>(replaceDataSize));
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
        printFSBHeaderIndexes(argv[1]);
        // outputAudioFiles(argv[1]);
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

