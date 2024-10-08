#include "JIT.hpp"

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

using namespace llvm;
using namespace clang;
using namespace llvm::orc;

bool initialized = false;

JIT::JIT(const std::vector< std::string > & clang_flags) {

  if (!initialized) {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    initialized = true;
  }

  std::vector<const char *> flags = {
    "-march=native", "-Xclang", "-emit-llvm-only", 
    //"-stdlib=libc++",
    #ifdef __APPLE__
    "-isystem", 
    //"/opt/local/lib/clang/16/include",
    "/opt/homebrew/opt/llvm/lib/clang/18/include",

    "-isysroot",
    "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
    //"/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX14.4.sdk",
    //"-mmacosx-version-min=14.3"
    #endif
  };

  for (const auto & flag : clang_flags) {
    flags.push_back(flag.c_str());
  }

  IncrementalCompilerBuilder builder;
  builder.SetCompilerArgs(flags);

  auto CI = llvm::cantFail(builder.CreateCpp());
  interpreter = llvm::cantFail(Interpreter::create(std::move(CI)));

}

JIT::~JIT(){}

void JIT::compile(std::string code) {
  llvm::cantFail(interpreter->ParseAndExecute(code.c_str()));
}

llvm::orc::ExecutorAddr JIT::lookup_function_addr(std::string func_name) {
  return llvm::cantFail(interpreter->getSymbolAddress(func_name.c_str()));
}
