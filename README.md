# sm3tools (ALPHA)

# Description

A program that is currently limited to 
extracting and editing data in PCSSB archives 
from Spider-Man 3 The Game (PC).

## Program Modes

When you run the executable, there are a couple of "modes"
of operation you can choose between. 

- The default is **extract**, which just outputs
the files found within the specified input file to the output directory.
- Another is **list**, set by using the `--list` (or `-l`) flag,
which prints out a listing of files within the archive.
- Finally, there's **replace**, set by using the `--replace` (or `-r`) flag,
where you must simultaneously pass a path as a flag value
to specify the file to replace within the archive.

## Flags

`-i <arg> | --input <arg>` - recommended way to pass the path to an input file
`-r <arg> | --replace <arg>` - recommended way to pass the path to a
file to replace within the input file
`-o <arg> | --replace <arg>` - pass the path to the output directory (defaults to `./out`).
Does nothing if `--overwrite-input | -oi` is specified.
`--overwrite-input | -oi` - overwrites the input file (only works in replace mode)
`-v | --verbose` - verbose (currently unused)
`-l | --list` - list files in archive

### Positional Arguments

For convenience some arguments can be passed without using their respective flag
in certain situations. However it is still recommended that you do use the flag
version of the command because it parses correctly more consistently.

Positional arguments are only accepted at the very beginning, before any flags are passed.

Argument 1 - Input File Argument (equivalent to `--input <arg>` or `-i <arg>`)
Argument 2 - File to replace (equivalent to `--replace <arg>` or `-r <arg>`) (sets mode to replace)

## Example Workflow - PCSSB

1. Copy the path of a .pcssb file that you want to extract from the game's sound folder
2. Run `sm3tools -i <path to PCSSB file>` to extract the audio from it
3. View the extracted audio files in `./out`
4. Modify and replace the relevant audio file that you want to change
5. Inject the modified audio into the PCSSB by running 
`sm3tools -i <path to PCSSB file> -r <path to modified audio>`
6. Modified PCSSB will be placed in the default output directory (`./out`)

## Caveats

* Currently, this program does not allow replacing audio with higher file sizes than what is 
in the PCSSB. This is because doing so alters the offsets of FSB files after the file that is replaced,
which prevents them from playing properly in-game.
* The program does not currently validate the modified audio apart from checking its total size.

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