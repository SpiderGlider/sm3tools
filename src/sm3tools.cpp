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

#include "sm3tools.hpp"

#include <iostream>
#include <filesystem>
#include <string>

#include <cassert>
#include <cstdlib>

#include "pcssb.hpp"

FileType getFileType(const char *const filePath) {
    assert(filePath != nullptr);

    const std::string fileExtension { std::filesystem::path(filePath).extension().string() };

    if (fileExtension.empty()) {
        std::cerr << "ERROR: Argument doesn't have a file extension."
                        " Are you sure this is a path to a file?\n";
        std::exit(EXIT_FAILURE);
    }

    // TODO could check magic numbers as well
    // FIXME case sensitive currently
    if (fileExtension == ".PCPACK") return PCPACK;
    if (fileExtension == ".pcssb") return PCSSB;

    std::cerr << "ERROR: File extension not recognised.\n";
    std::exit(EXIT_FAILURE);
}

void printHelp() {
    std::cout << "Usage (1): sm3tools.exe <PCSSB File>\n";
    std::cout << "Usage (2): sm3tools.exe <Input PCSSB File> <Audio File To Replace>\n";
    std::cout << "(1) Outputs all audio files from the PCSSB into the out directory.\n";
    std::cout << "(2) Injects the specified audio file into the PCSSB file, replacing "
        "the file with the same name.\n";
}

// Program takes one argument, that being the path to a file to parse.
// Currently only PCSSB parsing is implemented. The file type is determined
// only through the file extension currently.
int main(const int argc, const char *const argv[]) {
    //vectorize arguments using the range constructor
    std::vector<std::string> args {argv, argv + argc };

    if (argc < 2) {
        std::cerr << "ERROR: No arguments specified.\n";
        printHelp();
        std::exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; i++) {
        if (args[i] == "--help") {
            printHelp();
            std::exit(EXIT_SUCCESS);
        }
    }

    if (argc > 3) {
        std::cerr << "WARNING: Arguments after the 2nd"
                        " argument are currently ignored.\n";
    }

    const FileType fileType { getFileType(argv[1]) };
    if (fileType == PCPACK) {
        std::cerr << "ERROR: PCPACK parsing is not yet implemented.\n";
        std::exit(EXIT_FAILURE);
    }
    // currently getFileType should never return a value outside of these two
    assert(fileType == PCSSB);
    std::cout << "INFO: Parsing as a PCSSB file.\n";

    if (argc == 2) {
        (void) std::printf("INFO: Extracting audio from %s\n", argv[1]);
        // printFSBHeaderIndexes(argv[1]);
        outputAudioFiles(argv[1]);
    }
    else {
        (void) std::printf("Replacing %s in %s\n", argv[2], argv[1]);
        replaceAudioinPCSSB(argv[1], argv[2]);
    }

    return EXIT_SUCCESS;
}
