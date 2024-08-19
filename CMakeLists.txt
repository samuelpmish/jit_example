cmake_minimum_required(VERSION 3.18)
project(example_jit_project LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Clang CONFIG REQUIRED clangTooling libClang clangASTMatchers)

add_library(jit SHARED include/JIT.hpp src/JIT.cpp)
target_include_directories(jit PUBLIC include)
target_include_directories(jit PUBLIC ${LLVM_INCLUDE_DIR})
target_include_directories(jit PUBLIC ${CLANG_INCLUDE_DIRS})

message("${LLVM_INCLUDE_DIR}")
message("${CLANG_INCLUDE_DIRS}")

#target_precompile_headers(jit PRIVATE include/pch.hpp)
target_link_libraries(jit PUBLIC
  clangAST
  clangBasic
  clangInterpreter
  clangFrontend
  clangSema
)

if (TARGET clangAST) 
  message("found clangAST")
endif()

if (TARGET clangBasic) 
  message("found clangBasic")
endif()

if (TARGET clangInterpreter) 
  message("found clangInterpreter")
endif()

if (TARGET clangFrontEnd) 
  message("found clangFrontEnd")
endif()

if (TARGET clangSema) 
  message("found clangSema")
endif()

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE jit)

add_executable(perf src/perf.cpp)
target_link_libraries(perf PRIVATE jit)

add_executable(input_file src/input_file.cpp)
target_link_libraries(input_file PRIVATE jit)
