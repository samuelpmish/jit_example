cmake_minimum_required(VERSION 3.18)
project(example_jit_project LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

#find_package(LLVM)
find_package(Clang REQUIRED clangTooling libClang clangASTMatchers)

add_executable(main main.cpp)

target_precompile_headers(main PUBLIC pch.hpp)

target_link_libraries(main PUBLIC
  clangAST
  clangBasic
  clangInterpreter
  clangFrontend
  clangSema
)

#get_property(idirs TARGET main PROPERTY )

message(STATUS "Interface include directory for main: ${CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES}")
