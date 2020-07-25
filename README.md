# Modern Modbus
A library for programming in 2020 with a protocol made in 1979 using a
C standard from 1999 and a C++ standard from 2014.

## Motivation
This library was created to provide a more intuitive interface to making a Modbus TCP server than
`libmodbus` while using new Linux features such as `epoll` to increase performance.

## Example
See [the example server](example/main.cpp) for the main usage features of the server.

## Standards Compliance
IDK how compliant the server is to the modbus spec, but it should be compliant enough for rocket project.

PRs welcome if you come across any non-compliance.

## Usage / Integration
The folder is a standard CMake project, so just put it in your repo and use `add_subdirectory` to add
the folder's targets to your own.

Check the dependency chain in `CMakeLists.txt` to make sure you're linking everything correctly.

The following three targets are included in the CMakeLists.txt:
* `modern-modbus-core`: A C99 compatible implementation of a modbus command parser (no framing support)
* `modern-modbus-tcp`: A C99 compatible implementation of a modbus tcp frame parser. Define `MODBUS_STATIC_BUFFERS`
in the preprocessor to enable pure static allocation mode.
* `modern-modbus-tcp-server`: A C++14 compatible implementation of a modbus TCP server (I couldn't take C anymore).
Behind the scenes it uses `epoll` to keep everything performant on a single thread, and a RAII magic to keep everything
memory safe.

## Testing
One program I found useful for testing the compliance of the server is 
[RMMS](http://en.radzio.dxp.pl/modbus-master-simulator/).