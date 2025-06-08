#include <iostream>
#include <filesystem>

#include <cassert>
#include <cstdlib>
#include <cstring>

typedef enum FILE_TYPE {
    PCPACK,
    PCSSB
} FileType;

// Checks the file extension of the file name, and returns the relevant
// file type if one is matched. Otherwise it exits with an error message
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

// Program takes one argument, that being the path to a file to parse.
// Currently only PCSSB parsing is implemented. The file type is determined
// only through the file extension currently.
int main(const int argc, const char *const argv[]) {
    if (argc < 2) {
        std::cerr << "ERROR: Must have 1 argument"
                        " (the path to a file to parse).\n";
        std::exit(EXIT_FAILURE);
    }
    if (argc > 2) {
        std::cerr << "WARNING: Arguments after the 1st"
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

    return EXIT_SUCCESS;
}
