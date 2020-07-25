cmake_minimum_required(VERSION 3.13)
project(modern_modbus)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

add_subdirectory(core)
add_subdirectory(tcp)
add_subdirectory(tcp_server)

target_link_libraries(modern-modbus-tcp modern-modbus-core)

target_include_directories(modern-modbus-tcp-server PUBLIC spdlog/include)
target_link_libraries(modern-modbus-tcp-server modern-modbus-core modern-modbus-tcp)

option(BUILD_EXAMPLE "set to ON if you want to build the example")
if(BUILD_EXAMPLE)
    add_subdirectory(spdlog)
    add_executable(modern-modbus-tcp-example example/main.cpp)
    target_link_libraries(modern-modbus-tcp-example modern-modbus-core modern-modbus-tcp modern-modbus-tcp-server spdlog::spdlog)
endif()
unset(BUILD_EXAMPLE CACHE)