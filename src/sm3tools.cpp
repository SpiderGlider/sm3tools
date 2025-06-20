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

FileType getFileType(const std::string_view filePath) {
    assert(!filePath.empty());

    const std::string fileExtension { std::filesystem::path(filePath).extension().string() };

    if (fileExtension.empty()) {
        return FileType::none;
    }

    // TODO could check magic numbers as well
    // FIXME case sensitive currently
    if (fileExtension == ".PCPACK") return FileType::pcpack;
    if (fileExtension == ".pcssb") return FileType::pcssb;

    return FileType::unknown;
}

const std::string& findInputFileArg(const std::vector<std::string>& args) {
    assert(!args.empty());

    for (size_t i = 1; i < args.size(); i++) {
        if (args[i] == "--input" || args[i] == "-i") {
            if ((i+1) < args.size()) {
                return args[i+1];
            }
            std::cerr << "ERROR: Input flag was passed, "
                    "but no input file was specified!\n";
        }
    }
    return args[1];
}

struct options {
    const bool list;
    const bool verbose;
    const std::string& inputFilePath;
    const FileType inputFileType;
    const std::string& replaceFilePath;
};

struct options parseFlags(const std::vector<std::string>& args) {
    bool list { false };
    bool verbose = { false };
    const std::string& inputFilePath { findInputFileArg(args) };

    for (size_t i = 1; i < args.size(); i++) {
        if (args[i] == "--list" || args[i] == "-l") {
            list = true;
        }
        else if (args[i] == "--verbose" || args[i] == "-v") {
            verbose = true;
        }
        else if (args[i] == "--") {

        }
        else if (args[i] == "--input" || args[i] == "-i") {
            if ((i+1) < args.size()) {
                const std::string& inputFilePath { args[i+1] };
            }
            std::cerr << "ERROR: Input flag was passed, "
                    "but no input file was specified!\n";
        }
    }

    return { list, verbose, inputFilePath}
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
    const std::vector<std::string> args {argv, argv + argc };
    assert(argc == static_cast<int>(args.size()));
    assert(args[0] == argv[0]);

    if (argc < 2) {
        std::cerr << "ERROR: No arguments specified.\n";
        printHelp();
        return EXIT_FAILURE;
    }

    for (size_t i = 1; i < args.size(); i++) {
        if (args[i] == "--help" || args[i] == "-h") {
            printHelp();
            return EXIT_SUCCESS;
        }
    }

    if (argc > 3) {
        std::cerr << "WARNING: Arguments after the 2nd"
                        " argument are currently ignored.\n";
    }

    const std::string& inputFilePath { findInputFileArg(args) };

    const FileType fileType { getFileType(inputFilePath) };
    if (fileType == FileType::none) {
        std::cerr << "ERROR: Argument doesn't have a file extension."
                                " Are you sure this is a path to a file?\n";
        return EXIT_FAILURE;
    }
    if (fileType == FileType::unknown) {
        std::cerr << "ERROR: File extension not recognised.\n";
        return EXIT_FAILURE;
    }
    if (fileType == FileType::pcpack) {
        std::cerr << "ERROR: PCPACK parsing is not yet implemented.\n";
        return EXIT_FAILURE;
    }
    // currently getFileType should never return a value outside of these
    assert(fileType == FileType::pcssb);
    std::cout << "INFO: Parsing as a PCSSB file.\n";

    for (size_t i = 1; i < args.size(); i++) {
        if (args[i] == "--list" || args[i] == "-l") {
            std::cout << "INFO: Listing FSBs in " << inputFilePath << '\n';
            printFSBHeaderIndexes(inputFilePath);
            return EXIT_SUCCESS;
        }
    }

    if (argc == 2) {
        std::cout << "INFO: Extracting audio from " << inputFilePath << '\n';
        outputAudioFiles(args[1]);
    }
    else {
        //FIXME
        std::cout << "Replacing " << args[2] << " in " << inputFilePath << '\n';
        replaceAudioinPCSSB(inputFilePath, args[2]);
    }

    return EXIT_SUCCESS;
}
