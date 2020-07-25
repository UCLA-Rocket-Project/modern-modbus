set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

add_library(modern-modbus-tcp-server Responder.h Responder.cpp Socket.h Socket.cpp TCPServer.cpp mb_exceptions.h)
target_include_directories(modern-modbus-tcp-server PUBLIC ${PROJECT_SOURCE_DIR}/include)
set_property(TARGET modern-modbus-tcp-server PROPERTY LANGUAGE CXX)