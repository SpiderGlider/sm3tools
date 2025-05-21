#ifndef PCSSB_H
#define PCSSB_H
#include <stddef.h>
#include <stdint.h>

struct FSB {
    uint32_t fsb3Header; // FSB3

    uint32_t numFiles; // = 1

    uint32_t unknown1; // = 80

    uint32_t dataSize;

    uint32_t unknown2; // = 196609
    uint32_t null1;

    char* fileName; // 32 bytes

    uint32_t unknown3;
    uint32_t unknown4;
    uint32_t null2;
    uint32_t unknown5;
    uint32_t unknown6; // = 8768
    uint32_t unknown7; // = 48000
    uint32_t unknown8; // = 1
    uint32_t unknown9; // = 131200
    uint32_t unknown10; // = 1065353216
    uint32_t unknown11; // = 1176256512
    uint32_t null3;
    uint32_t null4;

    char* data;
};

struct FSB readFile(const char* fileName);

size_t findFSBHeaderIndexes(
    const char *const inputFileName,
    size_t *const resultArr,
    const size_t resultArrLen);

void printFSBHeaderIndexes(const char *const fileName);

#endif
