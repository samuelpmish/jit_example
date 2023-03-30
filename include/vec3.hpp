#pragma once

#include <iostream>

struct vec3 {
  double x, y, z;
};

vec3 operator*(vec3 v, double scale) {
  return {v.x * scale, v.y * scale, v.z * scale};
}

std::ostream & operator<<(std::ostream & out, vec3 v) {
  out << "{" << v.x << " " << v.y << " " << v.z << "}";
  return out;
}