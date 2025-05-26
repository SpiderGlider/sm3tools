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

//Reads the data size field in the FSB file that starts at fsb3HeaderPosition.
//fsb3HeaderPosition must be the location of the start of the "FSB3" header string.
//This data size field is supposed to represent the length of the audio data embedded in
//the FSB3 file (i.e. the rest of the data in the file after the 104 bytes of header information).
//but it seems in the Spider-Man 3 PCSSB files this is not always the case.
uint32_t readDataSize(
    const char *const inputFileName,
    const size_t fsb3HeaderPosition);

//number of bytes used to store the sample filename in FSB archives,
//EXCLUDING the "P\0" at the start for simplicity
#define FSB_FILENAME_SIZE 30

//Reads the file name field in the FSB file that starts at fsb3HeaderPosition.
//fsb3HeaderPosition must be the location of the start of the "FSB3" header string.
//we ignore the "P\0" bytes at the start of the file name for simplicity.
void readFileName(
    const char *const inputFileName,
    const size_t fsb3HeaderPosition,
    char resultArr[FSB_FILENAME_SIZE]);

const int FSB_HEADER_SIZE = 104;

//Writes the audio data from an FSB file into a file with file name = outputFileName
//NOTE: overwrites file if it already exists.
void outputAudioData(
    const char *const inputFileName,
    const size_t fsb3HeaderPosition,
    const size_t headerSize,
    const size_t dataSize,
    const char *const outputFileName);

//Writes the audio data of all FSB files in a PCSSB into separate files.
//output file names are formatted as "[pcssb file name].pcssb-[FSB file name]".
//Assumes various things about the file that are likely only true for the Spider-Man 3
//PC .PCSSB files. For example, each FSB file is partly duplicated so we don't output the duplicate.
void outputAudioFiles(const char *const inputFileName);

#endif
