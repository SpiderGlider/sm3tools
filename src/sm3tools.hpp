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

#ifndef SM3TOOLS_H
#define SM3TOOLS_H
#include <string_view>
#include <string>
#include <vector>

enum class FileType {
    none,
    unknown,
    pcpack,
    pcssb,
};

// Checks the file extension of the file name, and returns the relevant
// file type if one is matched. Otherwise it returns unknown, or none if no
// file extension was found at all.
FileType getFileType(std::string_view filePath);

//Prints usage information to stdout
void printHelp();

// program options, which are decided depending on the flags the user passes
struct Options {
    bool help { false }; // whether to display usage information and exit
    bool list { false }; // whether to list files within the input archive and exit
    bool verbose { false }; // (UNUSED) whether to increase verbosity of output
    bool overwrite { false }; // whether to overwrite the original file
    std::string inputFilePath {}; // path to an input file archive
    std::string replaceFilePath {}; // path to a file to replace within the input archive
    // either the output directory (if outputting contents of archive),
    // or the output file path (if modifying an archive)
    std::string outputPath {};
};

//checks if a flag (either flagName or flagAltName) was passed at least once.
//flagAltName is a parameter so that you can check if either the short
//or long form of a flag was passed.
//flagName and flagAltName are case-sensitive.
bool checkFlagPresent(const std::vector<std::string>& args,
    const std::string_view flagName,
    const std::string_view flagAltName);

//find a positional argument by its arg number (starting from 1 for the first).
//for complexity reasons this does not include flags, and only arguments
//up until (but not including) the first flag are counted.
//returns the argument by value. if the argument is not found an empty
//string is returned.
std::string getArgument(const std::vector<std::string>& args,
    const size_t argNumber);

//looks for a value that was passed with the flag (either flagName or flagAltName).
//flagAltName is a parameter so that you can check if either the short
//or long form of a flag was passed.
//flagName and flagAltName are case-sensitive.
//if no value was passed with the flag or if the flag hasn't been passed at all
//an empty string is returned.
std::string getFlagValue(const std::vector<std::string>& args,
    const std::string_view flagName,
    const std::string_view flagAltName);

//wrapper function that aims to look for an argument which can be passed either
//as the value to a flag or as a positional argument (before any flags are passed).
//first the value to the flag (flagName or flagAltName) is checked, then if not found.
//the arg position argAltNumber is checked.
//flagAltName is a parameter so that you can check if either the short
//or long form of a flag was passed.
//flagName and flagAltName are case-sensitive.
//if not found as either, an empty string is returned.
std::string getArgOrFlagValue(const std::vector<std::string>& args,
    const std::string_view flagName,
    const std::string_view flagAltName,
    const size_t argAltNumber);

// parses program arguments to find any flags that are passed and construct
// the program options struct
Options parseFlags(const std::vector<std::string>& args);

// generates a default output file path given an input file.
// adds "-mod" onto the end of the stem (i.e. before the file extension).
// outputs in outputDirectory (./out)
std::string defaultModifiedFileOutPath(
    const std::string& inputFilePath,
    const std::string& outputDirectory);

// generates a temporary output file path given an input file.
// adds ".tmp" onto the end of the file path
std::string tempFileOutPath(const std::string& inputFilePath);

// performs operations on a PCSSB file using the specified program options
void pcssbMain(const Options& options);

#endif
