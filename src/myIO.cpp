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

#include "myIO.hpp"

#include <iostream>

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cerrno>

#ifdef _WIN32
#include <direct.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

namespace MyIO {
    void mkdir(const char *const path) {
        assert(path != nullptr);

#ifdef _WIN32
        const int returnValue = ::_mkdir(path);
#else
        const int returnValue = ::mkdir(path, 0700);
#endif
        if (returnValue != 0) {
            //Folder already existing is a natural thing to have occur
            //when running the program multiple times, so we don't need to log it
            if (errno != EEXIST) {
                std::perror("ERROR: Failed to create directory");
            }
        }
    }

    std::intmax_t getfilesize(const char *const filePath) {
        assert(filePath != nullptr);

#ifdef _WIN32
        struct _stat64 sb {};
        const int returnValue = ::_stat64(filePath, &sb);
        const intmax_t size = (intmax_t) sb.st_size;
#else
        struct stat sb {};
        const int returnValue = ::stat(filePath, &sb);
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

    std::FILE *fopen(const char *const fileName, const char *const mode) {
        assert(fileName != nullptr);
        assert(mode != nullptr);

        std::FILE *const fileHandle = std::fopen(fileName, mode);
        if (!fileHandle) {
            std::perror("ERROR: Failed to open file");
            std::exit(EXIT_FAILURE);
        }
        return fileHandle;
    }

    std::size_t fread(
        void *const buffer,
        const std::size_t size,
        const std::size_t count,
        std::FILE *const stream) {

        assert(buffer != nullptr);
        assert(stream != nullptr);
        assert(size > 0);
        assert(count > 0);

        const std::size_t objsRead = std::fread(buffer, size, count, stream);

        if (std::ferror(stream)) {
            std::perror("ERROR: I/O error when reading");
            (void) std::fclose(stream);
            std::exit(EXIT_FAILURE);
        }
        // TODO these logs have been left out for now, could be used when verbose is set?
        // if (std::feof(stream)) {
        //     std::cout << "LOG: EOF encountered when reading file.\n";
        // }
        // if (objsRead < count) {
        //     (void) std::printf("LOG: count was %zu, amount read was only %zu.\n", count, objsRead);
        // }

        return objsRead;
    }

    std::size_t fwrite(
        const void *const buffer,
        const std::size_t size,
        const std::size_t count,
        std::FILE *const stream) {

        assert(buffer != nullptr);
        assert(stream != nullptr);
        assert(size > 0);
        assert(count > 0);

        const std::size_t objsWritten = std::fwrite(buffer, size, count, stream);

        if (std::ferror(stream)) {
            std::perror("ERROR: I/O error when writing");
            (void) std::fclose(stream);
            std::exit(EXIT_FAILURE);
        }
        // if (objsWritten < count) {
        //     (void) std::printf("LOG: count was %zu, amount written was only %zu.\n", count, objsWritten);
        // }

        return objsWritten;
    }

    void fseek(std::FILE *const stream, const long int offset, const int origin) {
        assert(stream != nullptr);

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
    void fseekunsigned(std::FILE *const stream, const unsigned long int offset, const int origin) {
        if (offset > LONG_MAX){
            //call fseek with max value it supports for the offset
            MyIO::fseek(stream, LONG_MAX, origin);
            if (origin == SEEK_END) {
                //seeks backwards the remaining distance
                MyIO::fseek(stream, -(static_cast<long int>(offset - LONG_MAX)), SEEK_CUR);
            }
            else {
                //seeks forward the remaining distance
                MyIO::fseek(stream, static_cast<long int>(offset - LONG_MAX), SEEK_CUR);
            }
        }
        else {
            //fseek normally if below max supported value
            MyIO::fseek(stream, static_cast<long int>(offset), origin);
        }
    }
}
