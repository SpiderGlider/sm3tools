#ifndef PCSSB_H
#define PCSSB_H
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

struct FSB {
    std::uint32_t fsb3Header {}; // "FSB3"

    std::uint32_t numFiles {}; // = 1

    std::uint32_t unknown1 {}; // = 80

    std::uint32_t dataSize {};

    std::uint32_t unknown2 {}; // = 196609
    std::uint32_t null1 {};

    std::uint16_t entrySize {}; // = 80 ("P\0")

    char* fileName {}; // 30 bytes

    std::uint32_t unknown3 {};
    std::uint32_t unknown4 {};
    std::uint32_t null2 {};
    std::uint32_t unknown5 {};
    std::uint32_t unknown6 {}; // = 8768
    std::uint32_t unknown7 {}; // = 48000
    std::uint32_t unknown8 {}; // = 1
    std::uint32_t unknown9 {}; // = 131200
    std::uint32_t unknown10 {}; // = 1065353216
    std::uint32_t unknown11 {}; // = 1176256512
    std::uint32_t null3 {};
    std::uint32_t null4 {};

    char* data {};
};

// "FSB3" text that is at the start of each FSB file
constexpr std::string_view FSB_MAGIC_STRING { "FSB3" };

// "FSB3" text that is at the start of each FSB file, represented as
// an unsigned long.
constexpr std::uint32_t FSB_MAGIC_NUMBER { 859984710 }; // "FSB3"

//finds every instance of "FSB3" header text in the sound file
//puts the absolute position in bytes of those instances
//into the given resultArr in the order they are found.
//return value is number of results found. if it returns a value greater than resultArrLen,
//that means resultArr was too small and there may have been more results that couldn't fit.
//NOTE: we assume that the file is aligned in terms of longs (i.e. position of header in file
//is divisible by 4). In the case of Spider-Man 3 PCSSB files this seems to be the case.
std::size_t findFSBHeaderIndexes(
    const char *const inputFileName,
    std::size_t *const resultArr,
    const std::size_t resultArrLen);

//finds every instance of "FSB3" header text in the file given by the filePath,
//and puts the absolute position in bytes of the start of those instances
//(in order from the start of the file) into the vector that is returned.
//The size of the vector is the number of instances that were found.
std::vector<size_t> findFSBIndexes(
    const char *const filePath);

//prints out location of each instance of the text "FSB3" in the file.
//in format "[result number]: decimal = [address in decimal], hex = [address in hex]"
void printFSBHeaderIndexes(const char *const fileName);

constexpr int DATA_SIZE_OFFSET { 3 * sizeof(std::uint32_t) };

//Reads the data size field in the FSB file that starts at fsb3HeaderPosition.
//fsb3HeaderPosition must be the location of the start of the "FSB3" header string.
//This data size field is supposed to represent the length of the audio data embedded in
//the FSB3 file (i.e. the rest of the data in the file after the 104 bytes of header information).
//but it seems in the Spider-Man 3 PCSSB files this is not always the case.
std::uint32_t readDataSize(
    const char *const inputFileName,
    const std::size_t fsb3HeaderPosition);

//number of bytes used to store the sample filename in FSB archives,
//INCLUDING an extra byte (+1) for adding a null terminator at the end when
//read to a string, for simplicity. 30 + 1 = 31
constexpr int FSB_FILENAME_SIZE { 31 };

//NOTE: skips 6 longs which are the other header information including "FSB3" text
//before the entry starts, and then skips 2 bytes which is the entry size (which =80)
constexpr int FILENAME_OFFSET { 2 + (6 * sizeof(uint32_t)) };

//Reads the file name field in the FSB file that starts at fsb3HeaderPosition.
//fsb3HeaderPosition must be the location of the start of the "FSB3" header string.
//we ignore the "P\0" bytes at the start of the file name for simplicity.
void readFileName(
    const char *const inputFileName,
    const std::size_t fsb3HeaderPosition,
    char resultArr[FSB_FILENAME_SIZE]);

//find the first FSB in the PCSSB that has a filename field
//matching fileNameString. Returns the fsb header index of that FSB.
std::size_t findFirstFSBMatchingFileName(
    const char *const pcssbFileName,
    const char *const fileNameString);

constexpr int FSB_HEADER_SIZE { 104 };

//uses the filename of the file pointed to by replaceFilePath to find the relevant
//FSB that has a matching filename field. then replaces the audio data
//in that FSB with the contents of the file at replaceFilePath
//into a new file that has the filename of the file
//pointed to by pcssbFilePath but with "-mod" appended
//to the end.
void replaceAudioinPCSSB(
    const char *const pcssbFilePath,
    const char *const replaceFilePath);

//Writes the audio data from an FSB file into a file with file name = outputFileName
//NOTE: overwrites file if it already exists.
void outputAudioData(
    const char *const inputFileName,
    const std::size_t fsb3HeaderPosition,
    const std::size_t headerSize,
    const std::size_t dataSize,
    const char *const outputFileName);

//Writes the audio data of all FSB files in a PCSSB into separate files.
//output file names are formatted as "[pcssb file name].pcssb-[FSB file name]".
//Assumes various things about the file that are likely only true for the Spider-Man 3
//PC .PCSSB files. For example, each FSB file is partly duplicated so we don't output the duplicate.
void outputAudioFiles(const char *const inputFileName);

//reads readCount bytes from input (starting from readPosition)
//and writes what's read to the output file
//(destroying the file if it exists) if append is false.
//otherwise it appends to the output file.
//creates output file if it does not exist.
void readAndWriteToNewFile(
    const char *const inputFileName,
    const char *const outputFileName,
    const std::size_t readCount,
    const std::size_t readPosition,
    const bool append);

//replace a uint32_t field in a file.
//the field has to be exactly sizeof(uint32_t) bytes, any less or more
//and the write will not work as expected.
void replaceLongInFile(
    const char *const fileName,
    const std::size_t longPosition,
    const std::uint32_t newValue);

#endif
