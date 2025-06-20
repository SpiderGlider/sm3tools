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

namespace File {
    enum FileType {
        unknown,
        pcpack,
        pcssb,
    };
}


// Checks the file extension of the file name, and returns the relevant
// file type if one is matched. Otherwise it exits with an error message
File::FileType getFileType(std::string_view filePath);

//Prints usage information to stdout
void printHelp();

#endif
