#pragma once

#include <memory>
#include <string>
#include <vector>

#include "llvm/ExecutionEngine/Orc/Core.h"

namespace clang {
  class Interpreter;
};

struct JIT {
  // initialize with a list of flags to pass to `clang`
  JIT(const std::vector<std::string> & flags);
  ~JIT();

  // initialize with a list of flags to pass to `clang`
  void compile(std::string code);

  template <typename T>
  T lookup_function(std::string func_name) {
    return lookup_function_addr(func_name).toPtr<T>();
  }

 private:
  llvm::orc::ExecutorAddr lookup_function_addr(std::string func_name);

  std::unique_ptr<clang::Interpreter> interpreter;
};
