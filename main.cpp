//=== unittests/CodeGen/IncrementalProcessingTest.cpp - IncrementalCodeGen ===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Interpreter/Interpreter.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/Parser.h"
#include "clang/Sema/Sema.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/TargetParser/Triple.h"

#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include <memory>
#include <iostream>

//#include "JIT.hpp"

using namespace llvm;
using namespace clang;
using namespace llvm::orc;

ExitOnError ExitOnErr;

const char hello_program[] = 
R"(
#include <cstdio>
extern "C" void hello() {
  printf("hello world\n");
}
)";

const char noop_program[] = 
R"(
extern "C" void noop() {}
)";

const char add1_program[] = 
R"(
extern "C" double add1(double x) {
  return x + 1;
}
)";

int main() {

  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();

  std::vector<const char *> ClangArgv = {
    "-Xclang", "-emit-llvm-only",
    "-arch", "arm64",
    "-isysroot", "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
  };
  auto CI = llvm::cantFail(IncrementalCompilerBuilder::create(ClangArgv));
  auto Interp = llvm::cantFail(Interpreter::create(std::move(CI)));
  
  std::cout << "parsing program" << std::endl;
  llvm::cantFail(Interp->ParseAndExecute(add1_program));

  std::cout << "get symbol address" << std::endl;
  auto add1_address = ExitOnErr(Interp->getSymbolAddress("add1"));

  std::cout << "convert to function pointer" << std::endl;
  double (*add1)(double) = jitTargetAddressToFunction< double(*)(double) >(add1_address);

  std::cout << "calling add1(3.0)" << std::endl;
  std::cout << add1(3.0) << std::endl;
 
  return 0;
}
