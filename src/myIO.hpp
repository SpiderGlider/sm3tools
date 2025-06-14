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

#ifndef MYIO_H
#define MYIO_H
#include <cstdint>
#include <cstdio>

namespace MyIO {
    //cross-platform mkdir wrapper.
    //creates directory with mode 0777 on linux
    //logs the error if the directory failed to be created
    void mkdir(const char *path);

    //cross-platform stat wrapper.
    //logs the error and exits if performing stat fails
    std::intmax_t getfilesize(const char *path);

    //wrapper functions around the <stdio.h> I/O functions
    //with additional logging/checks

    //wrapper around fopen that checks if the returned
    //pointer is NULL, in which case it prints the error to stderr and exits.
    //NOTE: this does not close the file handle so you have to call fclose
    //after you're done using it, like with normal fopen.
    std::FILE *fopen(const char *fileName, const char *mode);

    //wrapper around fread that checks feof and ferror
    //after doing so. Prints to the terminal if any of those happen, and exits
    //in the case of ferror. The number of objects read is checked to see whether
    //it matches count, but it is still returned in case the caller wants to use it
    //for e.g. loop conditions
    std::size_t fread(
        void *buffer,
        std::size_t size,
        std::size_t count,
        std::FILE *stream);

    //wrapper around fwrite that checks ferror after calling it.
    //if there is an error it is printed and then the program exits.
    //The number of objects written is checked to see whether
    //it matches count, but it is still returned in case the caller wants to use it
    //for e.g. loop conditions
    std::size_t fwrite(
        const void *buffer,
        std::size_t size,
        std::size_t count,
        std::FILE *stream);

    //wrapper around fseek that checks whether the return
    //value is non-zero, in which case it prints to stderr and exits.
    void fseek(std::FILE *stream, long int offset, int origin);

    //fseek but working with unsigned long values. accounts for values over what
    //signed longs support by seeking twice.
    //if first fseek fails second isn't executed.
    void fseekunsigned(std::FILE *stream, unsigned long int offset, int origin);
}
#endif
