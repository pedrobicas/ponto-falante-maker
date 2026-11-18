#pragma once

#include <cstdint>

class TwoWire {
 public:
  bool begin() { return true; }
  bool setClock(uint32_t) { return true; }
};
extern TwoWire Wire;
