set(CMAKE_C_STANDARD 99)

add_library(modern-modbus-tcp tcp_parse.c stringify.c tcp_response.c)
target_include_directories(modern-modbus-tcp PUBLIC ${PROJECT_SOURCE_DIR}/include)
set_property(TARGET modern-modbus-tcp PROPERTY LANGUAGE C)