// Display simulado: guarda o texto do último display() em sim::tela.
#pragma once

#include <string>
#include <vector>

#include "Wire.h"
#include "sim.h"

#define SSD1306_SWITCHCAPVCC 0x02
#define SSD1306_WHITE 1

class Adafruit_SSD1306 {
 public:
  Adafruit_SSD1306(uint8_t w, uint8_t h, TwoWire* twi, int8_t rst) { (void)w; (void)h; (void)twi; (void)rst; }
  bool begin(uint8_t vcc, uint8_t endereco) { (void)vcc; return endereco == 0x3C; }
  void clearDisplay() { buffer_.clear(); }
  void display() { sim::tela = buffer_; }
  void setTextColor(uint16_t) {}
  void setTextWrap(bool) {}
  void setTextSize(uint8_t) {}
  void setCursor(int16_t, int16_t) {}
  size_t print(const char* s) { buffer_.push_back(s); return buffer_.back().size(); }

 private:
  std::vector<std::string> buffer_;
};
