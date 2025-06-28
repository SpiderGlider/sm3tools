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

//finds the input file path from the program arguments.
//if the -i flag isn't passed, we assume that the file path is the 1st argument.
const std::string& findInputFileArg(const std::vector<std::string>& args);

// program options, which are decided depending on the flags the user passes
struct Options {
    bool help { false };
    bool list { false };
    bool verbose { false };
    std::string inputFilePath {};
    FileType inputFileType { FileType::unknown };
    std::string replaceFilePath {};
};

// parses program arguments to find any flags that are passed and construct
// the program options struct
Options parseFlags(const std::vector<std::string>& args);

#endif
