# JIT sandbox
playing with `clang-repl` and `ORC`.

Note: this project is not meant for use in production, it was intended to prototype JIT compilation. Right now, the compilation flags from CMake are not being propagated into the `JIT` class, so users currently have to specify certain include directories manually.

# Build
This depends on llvm 18.0 with the `clang-tools-extra` project enabled. llvm was built with the following configure arguments:

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

# Examples

## 1. Function template instantiation

[main.cpp](https://github.com/samuelpmish/jit_sandbox/blob/main/src/main.cpp) shows a basic example of how to use the `JIT` class that wraps `clang-repl`

1. The `JIT` constructor takes a list of compiler flags to be passed to the `clang-repl` interpreter:
```cpp
  JIT jit({"-O3"});
```
      
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

## 2. Optimization flags

[perf.cpp](https://github.com/samuelpmish/jit_sandbox/blob/main/src/perf.cpp) shows that passing optimization/architecture
compilation flags works just like `clang`. Functions compiled in the interpreter with `-O3` perform better than those with `-O0`.

## 3. Mixing AoT and JiT compilation

[input_file.cpp](https://github.com/samuelpmish/jit_sandbox/blob/main/src/input_file.cpp) shows an example of
compiling code defined in an auxiliary `json` file that is read at runtime. This allows for users to customize
the execution of a program without recompiling it.
