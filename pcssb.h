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

//finds every instance of "FSB3" header text in the sound file
//puts the absolute position in bytes of those instances
//into the given resultArr in the order they are found.
//return value is number of results found. if it returns a value greater than resultArrLen,
//that means resultArr was too small and there may have been more results that couldn't fit.
//NOTE: we assume that the file is aligned in terms of longs (i.e. position of header in file
//is divisible by 4). In the case of Spider-Man 3 PCSSB files this seems to be the case.
size_t findFSBHeaderIndexes(
    const char *const inputFileName,
    size_t *const resultArr,
    const size_t resultArrLen);

//prints out location of each instance of the text "FSB3" in the file.
//in format "[result number]: decimal = [address in decimal], hex = [address in hex]"
void printFSBHeaderIndexes(const char *const fileName);

//Outputs the audio data in folder structure matching input file name.
//e.g. for file "a.wav" in "b.PCSSB" the output will be in b/a.wav
void outputAudioData(
    const char *const inputFileName,
    const size_t fsb3HeaderPosition);

#endif
