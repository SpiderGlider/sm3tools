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

#ifndef PCPACK_H
#define PCPACK_H

#include <string>
#include <vector>

#include <cstddef>
#include <cstdint>

struct PCPACK {
    std::uint32_t unknown1 {}; // 26
    std::uint32_t unknown2 {}; // 2816
    std::uint32_t unknown3 {}; // 2658
    std::uint32_t unknown4 {}; // 849
    std::uint32_t unknown5 {}; // 411
    std::uint32_t unknown6 {}; // 4 / 0
    std::uint32_t unknown7 {}; // 48

    std::uint32_t dataOffset {};

    std::uint32_t unknown8 {};
    std::uint32_t null1 {};
    std::uint32_t null2 {};
    std::uint32_t null3 {};
    std::uint32_t null4 {};

    std::uint64_t header {}; // 1311768466702824296 = hsamxV4 + (byte)18

    std::uint32_t unknown9 {};
    std::uint32_t unknown10 {};
    std::uint32_t null5 {};
    std::uint32_t numOfNulls {}; // 11 / 1
    std::uint32_t unknown11 {}; // 1
    std::uint32_t unknown12 {}; // 16 / 8
    std::uint32_t null6 {};

    std::uint32_t numOfFiles {};

    std::uint32_t fileHash {};
    std::uint32_t fileType {};
    std::uint32_t fileOffset {};
    std::uint32_t fileSize {};
};

#endif
