# JIT sandbox
playing with `clang-repl` and `ORC`

# Build
This depends on llvm 16.0 with the `clang-tools-extra` project enabled. llvm was built with the following configure arguments:

```
cmake -G Ninja                                              \
      -DLLVM_TARGETS_TO_BUILD="host"                        \
      -DLLVM_ENABLE_PROJECTS="clang;lld;clang-tools-extra"  \
      -DCMAKE_BUILD_TYPE=Release                            \
      -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi"             \
```

-------

From there, configure and build normally:

```
cmake . -B build -DCMAKE_BUILD_TYPE=Release < other flags >
cmake --build build --parallel
```

# Overview

[main.cpp](https://github.com/samuelpmish/jit_sandbox/blob/main/src/main.cpp) shows a basic example of how to use the `JIT` class that wraps `clang-repl`

1. The `JIT` constructor takes a list of compiler flags to be passed to the `clang-repl` interpreter:
```cpp
  // flags for an M1 mac
  JIT jit({"-O3"});
```
TODO: see if I can get some of these default flags from CMake, so the user doesn't have to include
      platform-specific flags in their sources
      
--------

2. Make calls to `JIT::compile()` to add code snippets to the interpreter's "translation unit"

```cpp
  // define a function template
  jit.compile(R"(
    template < int m > 
    double foo() {
      return m + 2;
    }
  )");

  // instantiate a specialization of that function template, and wrap it in a C function
  jit.compile(R"(
    extern "C" {
      double foo_specialization2() {
        return foo<2>();
      }
    }
  )");
```

-------

3. Use `JIT::lookup_function` to get a function pointer to a function in the interpreter, and call it

```cpp
  double (*foo2)() = jit.lookup_function<double(*)()>("foo_specialization2");
  std::cout << foo2() << std::endl;
  // prints 4
```

notes:
- the type of the function pointer signature is not checked for consistency against the actual function
  signature in the interpreter (e.g. `jit.lookup_function<void(*)()>("foo_specialization2");` may compile but
  fail at runtime)
- function lookup is done by mangled name. Wrapping functions definitions in the interpreter with `extern "C"`
  makes lookup easier.
