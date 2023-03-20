cmake_minimum_required(VERSION 3.18)
project(example_jit_project LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

#find_package(LLVM)
find_package(Clang REQUIRED clangTooling libClang clangASTMatchers)

add_library(jit STATIC JIT.hpp JIT.cpp)
target_precompile_headers(jit PRIVATE pch.hpp)
target_link_libraries(jit PUBLIC
  clangAST
  clangBasic
  clangInterpreter
  clangFrontend
  clangSema
)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE jit)
add_executable(perf perf.cpp)
target_link_libraries(perf PRIVATE jit)



