#pragma once

#include <string>

struct BodyValue {
  BodyValue(std::string name, float value) : name(name), value(value) {}
  std::string name;
  float value;
};
