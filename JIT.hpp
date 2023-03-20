#pragma once

#include <vector>
#include <string>
#include <memory>

namespace clang {
  class Interpreter;
};

struct JIT {
  JIT(std::vector< std::string > flags);
  ~JIT();

  void compile(std::string code);

  template < typename T >
  T lookup_function(std::string func_name) {
    return reinterpret_cast< T >(lookup_function_addr(func_name));
  }

  private:
    uint64_t lookup_function_addr(std::string func_name);

    std::unique_ptr< clang::Interpreter > interpreter;
};