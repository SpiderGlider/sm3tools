/*
 * Copyright (c) 2025 SpiderGlider
 *
 * This file is part of sm3tools.
 *
 * sm3tools is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * sm3tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with sm3tools. If not, see <https://www.gnu.org/licenses/>.
 */

#include "pcssb.hpp"

#include <iostream>
#include <sstream>
#include <filesystem>
#include <array>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "myIO.hpp"

std::vector<size_t> findFSBIndexes(const std::string& filePath) {
    assert(!filePath.empty());

    std::vector<size_t> fsbIndexes {};
    //PCSSBs can have a lot more FSBs than this (though seldom more than 50),
    //but this expands in a way that should minimise the number of reallocations
    fsbIndexes.reserve(12);

    const auto fileSize = static_cast<size_t>(MyIO::getfilesize(filePath.c_str()));
    char *const buffer = new char[fileSize];
    {
        //NOTE: we assume that result of getfilesize is the actual file size
        std::FILE *const fileHandle { MyIO::fopen(filePath.c_str(), "rb") };
        {
            //read entire file into buffer
            const std::size_t numRead = MyIO::fread(
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

void printFSBHeaderIndexes(const std::string& filePath) {
    assert(!filePath.empty());

    const std::vector<size_t> indexes { findFSBIndexes(filePath) };
    for (std::size_t i = 0; i < indexes.size(); i += 2) {
        if (i < indexes.size() - 2) {
            const std::string fsbFileName = readFileName(filePath, indexes.at(i));
            std::printf("%zu: "
                        "file name %s, "
                        "hex address = 0x%zX, "
                        "fsb size = %zu, "
                        "duplicate size = %zu, "
                        "total size including duplicate = %zu\n",
                        i+1,
                        fsbFileName.c_str(),
                        indexes.at(i),
                        indexes.at(i+1)-indexes.at(i),
                        indexes.at(i+2)-indexes.at(i+1),
                        indexes.at(i+2)-indexes.at(i));
        }
    }
}

std::uint32_t readDataSize(
    const std::string& inputFileName,
    const std::size_t fsb3HeaderPosition) {

    assert(!inputFileName.empty());

    std::uint32_t dataSize { 0 };

    std::FILE *const fileHandle { MyIO::fopen(inputFileName.c_str(), "rb") };
    {
        //set the file position indicator to start of FSB file
        MyIO::fseekunsigned(fileHandle, fsb3HeaderPosition, SEEK_SET);
        //move to location where data size is written
        MyIO::fseek(fileHandle, DATA_SIZE_OFFSET, SEEK_CUR);

        //read data size long
        (void) MyIO::fread(&dataSize, sizeof(uint32_t), 1, fileHandle);
    }
    (void) std::fclose(fileHandle);

    return dataSize;
}

std::string readFileName(
    const std::string& inputFileName,
    const std::size_t fsb3HeaderPosition) {

    assert(!inputFileName.empty());

    //NOTE: an extra byte is added to the length for the null terminator
    std::array<char, FSB_FILENAME_SIZE + 1> buffer {};

    std::FILE *const fileHandle { MyIO::fopen(inputFileName.c_str(), "rb") };
    {
        //set the file position indicator to start of FSB file
        MyIO::fseekunsigned(fileHandle, fsb3HeaderPosition, SEEK_SET);
        //move to location where file name is written
        MyIO::fseek(fileHandle, FILENAME_OFFSET, SEEK_CUR);

        //read file name
        (void) MyIO::fread(buffer.data(), sizeof(char), FSB_FILENAME_SIZE, fileHandle);
    }
    (void) std::fclose(fileHandle);
    //NOTE: we add a null terminator manually
    //for the case that the file name is 30 bytes long.
    buffer[FSB_FILENAME_SIZE] = '\0';

    //NOTE: we don't include the length in this constructor because
    //that would tell it to include trailing null characters rather
    //than stopping when it encounters the null terminator
    return { buffer.data() };
}

void outputAudioData(
    const std::string& inputFileName,
    const std::size_t fsb3HeaderPosition,
    const std::size_t headerSize,
    const std::size_t dataSize,
    const std::string& outputFileName) {

    assert(!inputFileName.empty());
    assert(!outputFileName.empty());

    char *const audioData { new char[dataSize] };
    {
        std::FILE *const inputFileHandle { MyIO::fopen(inputFileName.c_str(), "rb") };
        {
            //set the file position indicator to start of FSB file
            MyIO::fseekunsigned(inputFileHandle, fsb3HeaderPosition, SEEK_SET);
            //move to start of audio data
            MyIO::fseekunsigned(inputFileHandle, headerSize, SEEK_CUR);

            //read audio data
            (void) MyIO::fread(audioData, sizeof(char), dataSize, inputFileHandle);
        }
        (void) std::fclose(inputFileHandle);

        //write it to the output file
        std::FILE *const outputFileHandle { MyIO::fopen(outputFileName.c_str(), "wb") };
        {
            (void) MyIO::fwrite(audioData, sizeof(char), dataSize, outputFileHandle);
        }
        (void) std::fclose(outputFileHandle);
    }
    delete[] audioData;
}

void outputAudioFiles(const std::string& inputFileName, const std::string_view outputDirectory) {
    assert(!inputFileName.empty());

    const std::vector<std::size_t> fsbIndexes { findFSBIndexes(inputFileName) };

    const std::filesystem::path inputFileNamePath = { inputFileName };

    const std::filesystem::path fileName { inputFileNamePath.filename() };
    if (fileName.empty()) {
        std::cerr << "ERROR: Input file doesn't have a file extension!\n";
        std::exit(EXIT_FAILURE);
    }
    const std::filesystem::path parentPath { inputFileNamePath.parent_path() };

    std::ostringstream stringStream {};
    //create output directory
    stringStream << outputDirectory;
    MyIO::mkdir(stringStream.str().c_str());

    //create directory for the PCSSB file within the output directory
    //NOTE: if output directory already has a / at the end, it shouldn't matter
    //because repeated directory separators are treated the same as a single one
    //TODO: test on windows if user inputs a directory with \ at the end
    stringStream << '/' << fileName.string();
    const std::string outputDirectoryPath { stringStream.str() };
    MyIO::mkdir(outputDirectoryPath.c_str());

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
        const std::string fsbFileName = readFileName(inputFileName, fsbIndexes[i]);

        //create path with file fsbFileName inside the output directory
        constexpr int OUTPUT_PATH_SIZE { 300 };
        char outputPath[OUTPUT_PATH_SIZE] {};
        (void) std::snprintf(
            outputPath,
            OUTPUT_PATH_SIZE,
            "%s/%s",
            outputDirectoryPath.c_str(),
            fsbFileName.c_str());
        outputAudioData(
            inputFileName,
            fsbIndexes[i],
            FSB_HEADER_SIZE,
            fsbDataSize,
            outputPath);
    }
}

std::size_t findFirstFSBMatchingFileName(
    const std::string& pcssbFileName,
    const std::string& fileNameString) {

    assert(!pcssbFileName.empty());
    assert(!fileNameString.empty());

    const std::vector<std::size_t> fsbIndexes = findFSBIndexes(pcssbFileName);

    for (const size_t fsbIndex : fsbIndexes) {
        if (readFileName(pcssbFileName, fsbIndex) == fileNameString) {
            return fsbIndex;
        }
    }

    std::cerr << "ERROR: File not found in PCSSB!\n";
    std::exit(EXIT_FAILURE);
}

void readAndWriteToNewFile(
    const std::string& inputFileName,
    const std::string& outputFileName,
    const size_t readCount,
    const size_t readPosition,
    const bool append,
    const bool padWithZeroes) {

    assert(!inputFileName.empty());
    assert(!outputFileName.empty());
    assert(readCount > 0);

    //store bytes from input in intermediate buffer
    //(plus one extra byte for null terminator)
    //NOTE: buffer is initialized to 0 to ensure that the bytes of the it that don't get read to
    // can still be written to the file as "zero padding" if padWithZeroes is enabled.
    char *const buffer { new char[readCount + 1]{} };
    {
        std::FILE *const inputFileHandle { MyIO::fopen(inputFileName.c_str(), "rb") };
        {
            MyIO::fseekunsigned(inputFileHandle, readPosition, SEEK_SET);

            const std::size_t numRead { MyIO::fread(buffer, sizeof(char), readCount, inputFileHandle) };

            //if padWithZeroes is false, only write the bytes that have been read
            //if padWithZeroes is true, write the entire length of the buffer,
            // with the remaining bytes being 00
            const std::size_t numToWrite { padWithZeroes ? readCount : numRead };

            assert(numToWrite <= readCount);
            //null terminate buffer string for safety
            buffer[numToWrite] = '\0';

            //either append or write depending on append argument
            //NOTE: we create an outputMode variable this way so that
            // we can keep outputFileHandle const
            const char *const outputMode { append ? "ab" : "wb" };
            std::FILE *const outputFileHandle { MyIO::fopen(outputFileName.c_str(), outputMode) };
            {
                (void) MyIO::fwrite(buffer, sizeof(char), numToWrite, outputFileHandle);
            }
            (void) std::fclose(outputFileHandle);
        }
        (void) std::fclose(inputFileHandle);
    }
    delete[] buffer;
}

void replaceLongInFile(
    const std::string& fileName,
    const std::size_t longPosition,
    const std::uint32_t newValue) {

    std::FILE *const fileHandle { MyIO::fopen(fileName.c_str(), "r+b") };
    {
        //move to long position
        MyIO::fseekunsigned(fileHandle, longPosition, SEEK_SET);

        //replace long
        const std::size_t numWritten { MyIO::fwrite(
            &newValue,
            sizeof(std::uint32_t),
            1,
            fileHandle) };

        if (numWritten != 1) {
            (void) std::fprintf(stderr, "ERROR: Error replacing long"
                            " at position %zu in %s!\n", longPosition, fileName.c_str());
            (void) std::fclose(fileHandle);
            std::exit(EXIT_FAILURE);
        }
    }
    (void) std::fclose(fileHandle);
}

void replaceAudioinPCSSB(
    const std::string& pcssbFilePath,
    const std::string& replaceFilePath) {

    //length of output path including null terminator
    //- byte for null terminator is included in sizeof("-mod")
    //TODO use a better output format
    const std::size_t outputFilePathSize { (std::strlen(pcssbFilePath.c_str()) + sizeof("-mod"))
        * sizeof(char) };
    char *const outputFilePath { new char[outputFilePathSize] };
    {
        //generate output file name
        (void) std::snprintf(outputFilePath, outputFilePathSize, "%s-mod", pcssbFilePath.c_str());

        //find audio file in PCSSB using its filename (including file extension but excluding path)
        //NOTE: we convert paths into strings first instead of using c_str() directly because the former
        //paths have a value type of wchar_t on windows and we need multi byte char c style strings.
        const std::string audioFileName { std::filesystem::path{replaceFilePath}.filename().string()};

        const std::size_t fsbHeaderIndex = findFirstFSBMatchingFileName(pcssbFilePath, audioFileName);
        const std::uint32_t originalDataSize = readDataSize(pcssbFilePath, fsbHeaderIndex);
        const std::intmax_t replaceDataSize = MyIO::getfilesize(replaceFilePath.c_str());

        if (static_cast<std::size_t>(replaceDataSize) > originalDataSize) {
            std::cerr << "ERROR: Given replacement audio has a larger file size than the original. "
                         "Inserting it into the PCSSB would result in undesirable side effects. Aborting.\n";
            std::exit(EXIT_FAILURE);
        }

        //TODO could trim metadata from the replacement audio

        const std::size_t fsbAudioDataIndex = fsbHeaderIndex + FSB_HEADER_SIZE;
        //write everything up to the existing audio data into the output files
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
            static_cast<std::size_t>(MyIO::getfilesize(pcssbFilePath.c_str())),
            fsbAudioDataIndex + originalDataSize,
            true,
            false);

        /* NOTE: we currently don't modify the data size field in the FSB because
            we only insert the replacement audio when it is smaller than
            or the same size as the original. in the former case, we use 00 for the
            remaining bytes - while technically they aren't really part of the audio
            so the value of the data size field won't actually be representative,
            in practise the game just ignores the null bytes and the original audio
            in some of the FSBs is formatted the same way anyway. */
    }
   delete[] outputFilePath;
}

