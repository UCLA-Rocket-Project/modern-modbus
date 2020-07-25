set(CMAKE_C_STANDARD 99)

add_library(modern-modbus-core write_cmd.c parse_cmd.c check_consistency.c stringify.c respond.c utils.c)
target_include_directories(modern-modbus-core PUBLIC ${PROJECT_SOURCE_DIR}/include)
set_property(TARGET modern-modbus-core PROPERTY LANGUAGE C)