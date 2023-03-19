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

const char add1_program[] = 
R"(
extern "C" double add1(double x) {
  return x + 1;
}
)";

int main(int argc, char *argv[]) {

  // Initialize LLVM.
  InitLLVM X(argc, argv);
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

#if 1
  PartialTranslationUnit & PTU = llvm::cantFail(Interp->Parse(add1_program));
  
  std::cout << "checking that add1() function exists" << std::endl;
  assert(PTU.TheModule->getFunction("add1"));

  std::cout << "get symbol address" << std::endl;
  auto add1_address = ExitOnErr(Interp->getSymbolAddress("_add1"));

  std::cout << "convert to function pointer" << std::endl;
  double (*add1)(double) = jitTargetAddressToFunction< double(*)(double) >(add1_address);

  std::cout << "calling add1(3.0)" << std::endl;
  std::cout << add1(3.0) << std::endl;

//  // Create an LLJIT instance.
//  auto J = ExitOnErr(LLJITBuilder().create());
//
//  auto Context = std::make_unique<LLVMContext>();
//  ThreadSafeModule ts_module(std::move(PTU.TheModule), std::move(Context));
//
//  ExitOnErr(J->addIRModule(std::move(ts_module)));
//
//  std::cout << "look up JIT'd function" << std::endl;
//  auto add1_fn = ExitOnErr(J->lookup("add1"));
//  double (*add1)(double) = add1_fn.toPtr<double(double)>();
//
//  std::cout << "calling add1(3.0)" << std::endl;
//  std::cout << add1(3.0) << std::endl;
#else
  PartialTranslationUnit & PTU = llvm::cantFail(Interp->Parse(hello_program));
  
  std::cout << "checking that hello() function exists" << std::endl;
  assert(PTU.TheModule->getFunction("hello"));

  // Create an LLJIT instance.
  auto J = ExitOnErr(LLJITBuilder().create());

  auto Context = std::make_unique<LLVMContext>();
  ThreadSafeModule ts_module(std::move(PTU.TheModule), std::move(Context));

  ExitOnErr(J->addIRModule(std::move(ts_module)));

  std::cout << "look up JIT'd function" << std::endl;
  auto hello_fn = ExitOnErr(J->lookup("hello"));
  void (*hello)() = hello_fn.toPtr<void()>();

  hello();

#endif
  
  return 0;
}
