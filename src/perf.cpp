#include <iostream>

#include "JIT.hpp"
#include "timer.hpp"

const char * func = R"(
  #include <vector>
  #include <cmath>
  extern "C" {
    int sum(int n) {
      int total = 0;
      for (int i = 0; i < n; i++) {
        total += i;
      }
      return total;
    }
  }
)";

int sum(int n) {
  int total = 0;
  for (int i = 0; i < n; i++) {
    total += i;
  }
  return total;
}
  
int main() {

  std::vector< double > values(10000, 1.0);
  timer stopwatch;

  JIT debug_jit({"-O0", "-g"});

  std::cout << "compiling unoptimized function" << std::endl;
  debug_jit.compile(func);

  std::cout << "get function pointer to specialization" << std::endl;
  auto debug_func = debug_jit.lookup_function<int(*)(int)>("sum");

  std::cout << "calling sum(100000)" << std::endl;
  stopwatch.start();
  double answer = debug_func(100000);
  stopwatch.stop();
  std::cout << answer << " " << stopwatch.elapsed() << std::endl;

  JIT opt_jit({"-O3"});

  std::cout << "compiling unoptimized function" << std::endl;
  opt_jit.compile(func);

  std::cout << "get function pointer to specialization" << std::endl;
  auto opt_func = opt_jit.lookup_function<int(*)(int)>("sum");

  std::cout << "calling optimized sum(100000)" << std::endl;
  stopwatch.start();
  answer = opt_func(100000);
  stopwatch.stop();
  std::cout << answer << " " << stopwatch.elapsed() << std::endl;

  return 0;
}
