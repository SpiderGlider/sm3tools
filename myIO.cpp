#include "myIO.hpp"

#include <iostream>

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <direct.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

void mymkdir(const char *const path) {
#ifdef _WIN32
    const int returnValue = _mkdir(path);
#else
    const int returnValue = mkdir(path, 0700);
#endif
    if (returnValue != 0) {
        std::perror("ERROR: Failed to create directory");
    }
}

std::intmax_t getfilesize(const char *const filePath) {
#ifdef _WIN32
    _stat64 sb {};
    const int returnValue = _wstat64(filePath, &sb);
    const intmax_t size = (intmax_t) sb.st_size;
#else
    struct stat sb {};
    const int returnValue = stat(filePath, &sb);
    const std::intmax_t size = sb.st_size;
#endif
    if (returnValue != 0) {
        std::perror("ERROR: Failed to get file size");
        std::exit(EXIT_FAILURE);
    }
    if (size < 0) {
        std::cerr << "WARNING: File size is negative,"
                        "it may or may not be handled correctly.\n";
    }
    return size;
}

std::FILE *myfopen(const char *fileName, const char *mode) {
    std::FILE *const fileHandle = fopen(fileName, mode);
    if (!fileHandle) {
        std::perror("ERROR: Failed to open file");
        std::exit(EXIT_FAILURE);
    }
    return fileHandle;
}

std::size_t myfread(
    void *const buffer,
    const std::size_t size,
    const std::size_t count,
    std::FILE *const stream) {

    //TODO will want to silence these kinds of logs for production...
    if (size == 0) {
        std::cout << "LOG: size of 0 was passed to fread!\n";
    }
    if (count == 0) {
        std::cout << "LOG: count of 0 was passed to fread!\n";
    }

    const std::size_t objsRead = std::fread(buffer, size, count, stream);

    if (std::ferror(stream)) {
        std::perror("ERROR: I/O error when reading");
        (void) std::fclose(stream);
        std::exit(EXIT_FAILURE);
    }
    if (std::feof(stream)) {
        std::cout << "LOG: EOF encountered when reading file.\n";
    }
    if (objsRead < count) {
        (void) std::printf("LOG: count was %lu, amount read was only %lu.\n", count, objsRead);
    }

    return objsRead;
}

std::size_t myfwrite(
    const void *const buffer,
    const std::size_t size,
    const std::size_t count,
    std::FILE *const stream) {

    const std::size_t objsWritten = std::fwrite(buffer, size, count, stream);

    if (std::ferror(stream)) {
        std::perror("ERROR: I/O error when writing");
        (void) std::fclose(stream);
        std::exit(EXIT_FAILURE);
    }
    if (objsWritten < count) {
        (void) std::printf("LOG: count was %lu, amount written was only %lu.\n", count, objsWritten);
    }

    return objsWritten;
}

void myfseek(std::FILE *const stream, const long int offset, const int origin) {
    const int returnValue = std::fseek(stream, offset, origin);
    if (returnValue != 0) {
        std::cerr << "ERROR: fseek() failed!\n";
        (void) std::fclose(stream);
        std::exit(EXIT_FAILURE);
    }
}

// credit to Tyler Durden on SO for this code, which is used with modifications.
// source: https://stackoverflow.com/a/47740105
// licensed under CC BY-SA 3.0
void myfseek_unsigned(std::FILE *const stream, const unsigned long int offset, const int origin) {
    if (offset > LONG_MAX){
        //call fseek with max value it supports for the offset
        myfseek(stream, LONG_MAX, origin);
        if (origin == SEEK_END) {
            //seeks backwards the remaining distance
            myfseek(stream, static_cast<long int>(-(offset - LONG_MAX)), SEEK_CUR);
        }
        else {
            //seeks forward the remaining distance
            myfseek(stream, static_cast<long int>(offset - LONG_MAX), SEEK_CUR);
        }
    }
    else {
        //fseek normally if below max supported value
        myfseek(stream, static_cast<long int>(offset), origin);
    }
}
