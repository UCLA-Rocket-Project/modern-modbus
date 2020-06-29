cmake_minimum_required(VERSION 3.13)
project(modern_modbus)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

include_directories(src)
add_library(modern-modbus-core src/Frame.h src/Frame.cpp src/FrameReader.h src/FrameReader.cpp src/Responder.h src/Responder.cpp src/ModbusError.h src/ModbusError.cpp src/mbutils.h)
add_library(modern-modbus-tcp src/ModbusTCPServer.h src/ModbusTCPServer.cpp src/ModbusTCPSocket.h src/ModbusTCPSocket.cpp)
target_link_libraries(modern-modbus-tcp modern-modbus-core)

option(BUILD_EXAMPLE "set to ON if you want to build the example")
if(BUILD_EXAMPLE)
    add_executable(modern-modbus-tcp-example example/main.cpp)
    target_link_libraries(modern-modbus-tcp-example modern-modbus-core modern-modbus-tcp)
endif()
unset(BUILD_EXAMPLE CACHE)