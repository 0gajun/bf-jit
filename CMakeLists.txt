cmake_minimum_required(VERSION 3.5.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_COMPILER "clang++")

project(bf-jit)
add_definitions("-std=c++11")
add_definitions("-Wall")
add_definitions("-O2")
add_definitions("-g")

set(SRC_COMMON utils.cpp bf_interp.cpp jit_utils.cpp)
set(ASMJIT_LIB ${CMAKE_SOURCE_DIR}/external/asmjit/build/libasmjit.a)

include_directories(${CMAKE_SOURCE_DIR}/external/asmjit/src)

add_executable(bf_simple ${SRC_COMMON} simple_interp.cpp)
target_compile_definitions(bf_simple PRIVATE SIMPLE)

add_executable(bf_opt1 ${SRC_COMMON} opt1_interp.cpp)
target_compile_definitions(bf_opt1 PRIVATE OPT1)

add_executable(bf_opt2 ${SRC_COMMON} opt2_interp.cpp)
target_compile_definitions(bf_opt2 PRIVATE OPT2)

add_executable(bf_opt3 ${SRC_COMMON} opt3_interp.cpp)
target_compile_definitions(bf_opt3 PRIVATE OPT3)

add_executable(bf_simple_jit ${SRC_COMMON} simple_jit.cpp)
target_compile_definitions(bf_simple_jit PRIVATE SIMPLE_JIT)

add_executable(bf_simple_asmjit ${SRC_COMMON} simple_asmjit.cpp)
target_link_libraries(bf_simple_asmjit ${ASMJIT_LIB})
target_compile_definitions(bf_simple_jit PRIVATE SIMPLE_JIT)
