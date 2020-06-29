# Modern Modbus
A library for programming in 2020 with a protocol made in 1979 using a
C++ standard from 2014.

## Motivation
This library was created to provide a more intuitive interface to making a Modbus TCP server than
`libmodbus` while using new Linux features such as `epoll` to increase performance.

## Example
See [the example server](example/main.cpp) for the main usage features of the server.

## Standards Compliance
IDK how compliant the server is to the modbus spec, but it should be compliant enough for rocket project.

PRs welcome if you come across any non-compliance.

## Usage
The folder is a standard CMake project, so just put it in your repo and use `add_subdirectory` to add
the folder's targets to your own.

The following two targets are included in the CMakeLists.txt:
* `modern-modbus-core`: this library contains the core modbus implementation and parser
* `modern-modbus-tcp`: this library contains the server implementation for Modbus/TCP 