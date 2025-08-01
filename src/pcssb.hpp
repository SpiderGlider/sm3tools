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

#ifndef PCSSB_H
#define PCSSB_H
#include <string>
#include <vector>

#include <cstddef>
#include <cstdint>

struct FSB {
    std::uint32_t fsb3Header {}; // "FSB3"

    std::uint32_t numFiles {}; // = 1

    std::uint32_t unknown1 {}; // = 80

    std::uint32_t dataSize {};

    std::uint32_t unknown2 {}; // = 196609
    std::uint32_t null1 {};

    std::uint16_t entrySize {}; // = 80 ("P\0")

    char* fileName {}; // 30 bytes

    std::uint32_t unknown3 {};
    std::uint32_t unknown4 {};
    std::uint32_t null2 {};
    std::uint32_t unknown5 {};
    std::uint32_t unknown6 {}; // = 8768
    std::uint32_t unknown7 {}; // = 48000
    std::uint32_t unknown8 {}; // = 1
    std::uint32_t unknown9 {}; // = 131200
    std::uint32_t unknown10 {}; // = 1065353216
    std::uint32_t unknown11 {}; // = 1176256512
    std::uint32_t null3 {};
    std::uint32_t null4 {};

    char* data {};
};

// "FSB3" text that is at the start of each FSB file
constexpr std::string_view FSB_MAGIC_STRING { "FSB3" };

//finds every instance of "FSB3" header text in the file given by the filePath,
//and puts the absolute position in bytes of the start of those instances
//(in order from the start of the file) into the vector that is returned.
//The size of the vector is the number of instances that were found.
std::vector<size_t> findFSBIndexes(const std::string& filePath);

//prints out information about each FSB (excluding duplicates) in the file.
void printFSBList(const std::string& filePath);

constexpr int DATA_SIZE_OFFSET { 3 * sizeof(std::uint32_t) };

//Reads the data size field in the FSB file that starts at fsb3HeaderPosition.
//fsb3HeaderPosition must be the location of the start of the "FSB3" header string.
//This data size field is supposed to represent the length of the audio data embedded in
//the FSB3 file (i.e. the rest of the data in the file after the 104 bytes of header information).
//but it seems in the Spider-Man 3 PCSSB files this is not always the case.
std::uint32_t readDataSize(
    const std::string& inputFileName,
    std::size_t fsb3HeaderPosition);

//maximum number of bytes used to store the sample filename in FSB3 archives
//NOTE that if the length of the file name takes the entire 30 bytes,
//an extra byte will be needed for the null terminator.
constexpr int FSB_FILENAME_SIZE { 30 };

//NOTE: skips 6 longs which are the other header information including "FSB3" text
//before the entry starts, and then skips 2 bytes which is the entry size (which =80)
constexpr int FILENAME_OFFSET { 2 + (6 * sizeof(uint32_t)) };

//Reads the file name field in the FSB file that starts at fsb3HeaderPosition.
//fsb3HeaderPosition must be the location of the start of the "FSB3" header string.
//we ignore the "P\0" bytes at the start of the file name for simplicity.
std::string readFileName(
    const std::string& inputFileName,
    std::size_t fsb3HeaderPosition);

//find the first FSB in the PCSSB that has a filename field
//matching fileNameString. Returns the fsb header index of that FSB.
std::size_t findFirstFSBMatchingFileName(
    const std::string& pcssbFileName,
    const std::string& fileNameString);

constexpr int FSB_HEADER_SIZE { 104 };

//uses the filename of the file pointed to by replaceFilePath to find the relevant
//FSB that has a matching filename field. then replaces the audio data
//in that FSB with the contents of the file at replaceFilePath
//into the file at outputFilePath. Creates the file if it does not exist, replaces
//it if it does exist.
void replaceAudioinPCSSB(
    const std::string& pcssbFilePath,
    const std::string& replaceFilePath,
    const std::string& outputFilePath);

//Writes the audio data from an FSB file into a file with file name = outputFileName
//NOTE: overwrites file if it already exists.
void outputAudioData(
    const std::string& inputFileName,
    std::size_t fsb3HeaderPosition,
    std::size_t headerSize,
    std::size_t dataSize,
    const std::string& outputFileName);

//Writes the audio data of all FSB files in a PCSSB into separate files.
//Written to a folder that has the name of the input file, in outputDirectory.
//Assumes various things about the file that are likely only true for the Spider-Man 3
//PC .PCSSB files. For example, each FSB file is partly duplicated so we don't output the duplicate.
void outputAudioFiles(const std::string& inputFileName, std::string_view outputDirectory);

//reads readCount bytes from input (starting from readPosition)
//and writes those bytes to the output file
//(destroying the file if it exists) if append is false.
//otherwise it appends to the output file.
//if padWithZeroes is true and the number of bytes that is read
//from the input ends up being less than readCount, readCount bytes
//are still written to the output (with null (00) bytes being written
//for the remaining bytes). otherwise, only the bytes that are
//read are written to the output, so the amount written may be less
//than readCount.
//creates output file if it does not exist.
void readAndWriteToNewFile(
    const std::string& inputFileName,
    const std::string& outputFileName,
    std::size_t readCount,
    std::size_t readPosition,
    bool append,
    bool padWithZeroes);

//replace a uint32_t field in a file.
//the field has to be exactly sizeof(uint32_t) bytes, any less or more
//and the write will not work as expected.
void replaceLongInFile(
    const std::string& fileName,
    std::size_t longPosition,
    std::uint32_t newValue);

#endif
