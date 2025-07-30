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
#include <vector>
#include <sstream>

#include <cassert>
#include <cstdlib>

#include "pcssb.hpp"

FileType getFileType(const std::string_view filePath) {
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

bool checkFlagPresent(const std::vector<std::string>& args,
    const std::string_view flagName,
    const std::string_view flagAltName) {

    for (size_t i = 1; i < args.size(); i++) {
        if (args[i] == flagName || args[i] == flagAltName) {
            return true;
        }
    }
    return false;
}

std::string getArgument(const std::vector<std::string>& args,
    const size_t argNumber) {

    assert(!args.empty());

    size_t argCount = 1;

    for (size_t i = 1; i < args.size(); i++) {
        if (args[i][0] == '-') {
            //if there is a flag preceding the argument, it would be
            //a complex procedure to determine whether the argument is part of
            //that flag or not, so we avoid that case altogether
            break;
        }

        if (argCount == argNumber) {
            return args[i];
        }
        argCount++;
    }

    return std::string {};
}

std::string getFlagValue(const std::vector<std::string>& args,
    const std::string_view flagName,
    const std::string_view flagAltName) {

    assert(!args.empty());

    for (size_t i = 1; i < args.size(); i++) {
        if (args[i] == flagName || args[i] == flagAltName) {
            if ((i + 1) < args.size()) {
                //NOTE that this doesn't check that
                //the next argument isn't a flag
                return args[i+1];
            }
        }
    }
    return std::string {};
}

std::string getArgOrFlagValue(const std::vector<std::string>& args,
    const std::string_view flagName,
    const std::string_view flagAltName,
    const size_t argAltNumber) {

    assert(!args.empty());

    //this being const would prevent automatic move
    std::string flagArgument = getFlagValue(args, flagName, flagAltName);

    if (flagArgument.empty()) {
        return getArgument(args, argAltNumber);
    }

    return flagArgument;
}

Options parseFlags(const std::vector<std::string>& args) {
    assert(!args.empty());

    const bool help { checkFlagPresent(args, "--help", "-h") };
    const bool list { checkFlagPresent(args, "--list", "-l") };
    const bool verbose = { checkFlagPresent(args, "--verbose", "-v") };
    const bool overwrite = { checkFlagPresent(args, "--overwrite-input", "-oi") };
    const std::string inputFilePath { getArgOrFlagValue(args, "--input", "-i", 1) };
    const std::string replaceFilePath { getArgOrFlagValue(args, "--replace", "-r", 2)};
    const std::string outputPath { getFlagValue(args, "--out", "-o") };

    return { help, list, verbose, overwrite, inputFilePath, replaceFilePath, outputPath };
}

void printHelp() {
    constexpr std::string_view USAGE_TEXT {
        "Usage (1): sm3tools.exe <Input PCSSB File> --list\n"
        "Usage (2): sm3tools.exe <Input PCSSB File> --out <Output Directory>\n"
        "Usage (3): sm3tools.exe <Input PCSSB File> --replace <Audio File To Replace> "
            "--out <Output Directory>\n"
        "(1) Prints out a listing of all the FSB files found within the PCSSB\n"
        "(2) Outputs all audio files from the PCSSB into the output directory\n"
        "(3) Injects the specified audio file into the PCSSB file, replacing "
            "it in the FSB with the same filename\n" };

    constexpr std::string_view FLAGS_TEXT {
        "FLAGS\n"
        //indented with four spaces
        "   -i <arg> | --input <arg> - Recommended way to pass the path to an input file\n"
        "   -r <arg> | --replace <arg> - Recommended way to pass the path to a "
            "file to replace within the input file\n"
        "   -o <arg> | --replace <arg> - Pass the path to the output directory \n"
        "       Defaults to ./out if not specified. Does nothing if `--overwrite-input | -oi` is specified\n"
        "   -oi | --overwrite-input - Overwrites the input file (only works in replace mode)\n"
        "   -v | --verbose - Increase verbosity (currently unused)\n"
        "   -l | --list` - List files in archive\n"
    };

    std::cout << USAGE_TEXT << '\n';
    std::cout << FLAGS_TEXT;
}

std::string defaultModifiedFileOutPath(const std::string& inputFilePath, const std::string& outputDirectory) {
    std::ostringstream strStream{};
    const std::filesystem::path inputFSPath { inputFilePath };
    //construct output file name
    strStream
        << inputFSPath.stem().string()
        << "-mod" 
        << inputFSPath.extension().string();
    const std::filesystem::path outputFileName { strStream.str() };
    return (outputDirectory / outputFileName).string();
}

std::string tempFileOutPath(const std::string& inputFilePath) {
    std::ostringstream strStream{};
    const std::filesystem::path inputFSPath { inputFilePath };
    //construct output file name
    strStream
        << inputFSPath.string()
        << ".tmp";
    return strStream.str();
}

void pcssbMain(const Options& options) {
    if (options.list) {
        std::cout << "INFO: Listing FSBs in " << options.inputFilePath << '\n';
        printFSBList(options.inputFilePath);
    }
    else if (!options.replaceFilePath.empty()) {
         std::cout << "Replacing " << options.replaceFilePath << " in " << options.inputFilePath << '\n';
         if (options.overwrite) {
             //output to a temporary file (input file name except with .tmp at the end)
             const std::string tempOutPath = tempFileOutPath(options.inputFilePath);

             replaceAudioinPCSSB(
                options.inputFilePath,
                options.replaceFilePath,
                tempOutPath);

             //replace the input file with the temporary file
             std::filesystem::rename(tempOutPath, options.inputFilePath);
         }
         else if (options.outputPath.empty()) {
             //default output path (input file name with -mod at the end of it, in the same directory)
             replaceAudioinPCSSB(
                 options.inputFilePath, 
                 options.replaceFilePath, 
                 defaultModifiedFileOutPath(options.inputFilePath, "./out"));
         }
         else {
             replaceAudioinPCSSB(options.inputFilePath, options.replaceFilePath, options.outputPath);
         }
    }
    else {
        std::cout << "INFO: Extracting audio from " << options.inputFilePath << '\n';
        if (options.outputPath.empty()) {
            outputAudioFiles(options.inputFilePath, "./out");
        }
        else {
            outputAudioFiles(options.inputFilePath, options.outputPath);
        }
    }
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

    const Options options { parseFlags(args) };

    if (options.help) {
        printHelp();
        return EXIT_SUCCESS;
    }

    if (options.inputFilePath.empty()) {
        std::cerr << "ERROR: Program needs an input file argument.\n";
        printHelp();
        return EXIT_FAILURE;
    }

    switch (getFileType(options.inputFilePath)) {
        case FileType::none:
            std::cerr << "ERROR: Argument doesn't have a file extension."
                                " Are you sure this is a path to a file?\n";
            return EXIT_FAILURE;
        case FileType::unknown:
            std::cerr << "ERROR: File extension not recognised.\n";
            return EXIT_FAILURE;
        case FileType::pcpack:
            std::cerr << "ERROR: PCPACK parsing is not yet implemented.\n";
            return EXIT_FAILURE;
        case FileType::pcssb:
            std::cout << "INFO: Parsing as a PCSSB file.\n";
            pcssbMain(options);
            break;
        default:
            std::cerr << "ERROR: Unhandled FileType!";
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
