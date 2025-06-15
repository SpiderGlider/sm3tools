# sm3tools

A program with tools for extracting various types of Spider-Man 3 the game (PC) files including PCSSB and PCPACK.

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