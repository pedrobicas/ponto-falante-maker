// Stub mínimo da API do Arduino-ESP32 (pacote esp32 3.x) para compilar e simular o firmware no PC.
// As assinaturas seguem cores/esp32/Arduino.h e esp32-hal-ledc.h.
#pragma once

#include <cmath>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>

#include "freertos_stub.h"
#include "sim.h"

#define HIGH 1
#define LOW 0
#define INPUT 0x01
#define OUTPUT 0x03
#define INPUT_PULLUP 0x05

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
uint16_t analogRead(uint8_t pin);
unsigned long millis();
void delay(uint32_t ms);
void delayMicroseconds(uint32_t us);
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L);
void tone(uint8_t pin, unsigned int frequency, unsigned long duration = 0);
void noTone(uint8_t pin);
bool ledcAttach(uint8_t pin, uint32_t freq, uint8_t resolution);
bool ledcWrite(uint8_t pin, uint32_t duty);

extern "C" bool getLocalTime(struct tm* info, uint32_t ms = 5000);
extern "C" void configTime(long gmtOffset_sec, int daylightOffset_sec, const char* server1,
                           const char* server2 = nullptr, const char* server3 = nullptr);

class String {
 public:
  String() = default;
  String(const char* s) : s_(s ? s : "") {}
  String(const std::string& s) : s_(s) {}
  const char* c_str() const { return s_.c_str(); }
  unsigned int length() const { return static_cast<unsigned>(s_.size()); }
  bool startsWith(const String& p) const { return s_.rfind(p.s_, 0) == 0; }
  bool operator==(const char* o) const { return s_ == o; }
  bool operator==(const String& o) const { return s_ == o.s_; }
  String operator+(const char* o) const { return String(s_ + o); }
  String operator+(const String& o) const { return String(s_ + o.s_); }
  const std::string& std() const { return s_; }

 private:
  std::string s_;
};

class HardwareSerial {
 public:
  void begin(unsigned long) {}
  int printf(const char* fmt, ...) __attribute__((format(printf, 2, 3)));
  size_t print(const char* s);
  size_t println(const char* s = "");
};
extern HardwareSerial Serial;
