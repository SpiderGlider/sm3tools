# sm3tools (ALPHA)

A program that can currently extract data from PCSSB archives from Spider-Man 3 The Game (PC).

## Building

### Visual Studio

Assuming you have the right C++ development packages installed,
Visual Studio should be able to detect the CMake project when 
loading the project folder.

### CMake

Create a directory for building inside the project folder, e.g. `build`.

Move into that directory, then in the terminal run:
```
cmake ..
```
to generate build files.

To run those build files you can use:
```
cmake --build .
```

## Flags

`-i <arg> | --input <arg>` - recommended way to pass the path to an input file
`-v | --verbose` - verbose (currently unused)
`-r <arg> | --replace <arg>` - recommended way to pass the path to a 
file to replace within the input file
`-l | --list` - list files in archive

## Example Workflow - PCSSB

1. Copy a .pcssb file that you want to extract from the game's sound folder
2. Run `sm3tools <path to copied PCSSB file>` to extract the audio from it
3. View the extracted audio files in `<directory of copied PCSSB file location>/out`
4. Modify the relevant audio file that you want to edit, keeping the file name the same
5. Inject the modified audio into the PCSSB by running 
`sm3tools <path to copied PCSSB file> <path to modified audio>`
6. Modified PCSSB will be where the copied PCSSB file is with "-mod" at the end of the file name

## Caveats

* Currently, this program does not allow replacing audio with higher file sizes than what is 
in the PCSSB. This is because doing so alters the offsets of FSB files after the file that is replaced,
which prevents them from playing properly in-game.
* The program does not currently validate the modified audio apart from checking its total size.