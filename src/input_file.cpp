#include "JIT.hpp"
#include "json.hpp"

#include "vec3.hpp"

#include <fstream>
#include <iostream>

using json = nlohmann::json;

std::string externC(std::string str) {
  return std::string("extern \"C\" {") + str + std::string("}");
}

int main() {
  JIT jit({"-I", "/Users/sam/code/jit_example/include"});

  jit.compile("#include \"vec3.hpp\"");

  std::cout << "compiling function defined in json file" << std::endl;
  json data = json::parse(std::ifstream("../input_file.json"));
  jit.compile(externC(data["body_force"]));

  vec3 (*body_force)(vec3) = jit.lookup_function<vec3(*)(vec3)>("body_force");

  std::cout << "calling body_force({1.0, 0.0, 0.0})" << std::endl;
  std::cout << body_force({1.0, 0.0, 0.0}) << std::endl;
 
  return 0;
}