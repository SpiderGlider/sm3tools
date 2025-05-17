#ifndef PCSSB_H
#define PCSSB_H
#include <stdint.h>

struct FSB {
    int32_t fsb3Header; // FSB3

    int32_t numFiles; // = 1

    int32_t unknown1; // = 80

    int32_t dataSize;

    int32_t unknown2; // = 196609
    int32_t null1;

    char* fileName; // 32 bytes

    int32_t unknown3;
    int32_t unknown4;
    int32_t null2;
    int32_t unknown5;
    int32_t unknown6; // = 8768
    int32_t unknown7; // = 48000
    int32_t unknown8; // = 1
    int32_t unknown9; // = 131200
    int32_t unknown10; // = 1065353216
    int32_t unknown11; // = 1176256512
    int32_t null3;
    int32_t null4;

    char* data;
};

#endif
