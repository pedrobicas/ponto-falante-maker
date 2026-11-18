// Implementação dos stubs: um "mundo" simulado controlado pelos testes.
#include <deque>
#include <iostream>

#include "Adafruit_SSD1306.h"
#include "Arduino.h"
#include "WebServer.h"
#include "WiFi.h"
#include "Wire.h"

namespace sim {
uint32_t agoraMs = 0;
float distanciaCm = -1;
int ldr = 3000;
bool botaoApertado = false;
int nivel[64] = {};
uint32_t pwm[64] = {};
unsigned tomHz = 0;
std::vector<unsigned> tons;
bool serialVisivel = false;
std::string serial;
std::vector<std::string> tela;
WebServer* servidorAtual = nullptr;

Resposta requisicao(const std::string& metodo, const std::string& caminho,
                    const std::map<std::string, std::string>& args) {
  return servidorAtual->chamar(metodo, caminho, args);
}
}  // namespace sim

HardwareSerial Serial;
WiFiClass WiFi;
TwoWire Wire;

void pinMode(uint8_t, uint8_t) {}
void digitalWrite(uint8_t pin, uint8_t val) { sim::nivel[pin] = val; }
int digitalRead(uint8_t pin) { return pin == 0 ? (sim::botaoApertado ? LOW : HIGH) : sim::nivel[pin]; }
uint16_t analogRead(uint8_t) { return static_cast<uint16_t>(sim::ldr); }
unsigned long millis() { return sim::agoraMs; }
void delay(uint32_t ms) { sim::agoraMs += ms; }
void delayMicroseconds(uint32_t) {}

unsigned long pulseIn(uint8_t, uint8_t, unsigned long timeout) {
  if (sim::distanciaCm < 0) return 0;
  const unsigned long us = static_cast<unsigned long>(sim::distanciaCm * 2.0f / 0.0343f);
  return us > timeout ? 0 : us;
}

void tone(uint8_t, unsigned int f, unsigned long) {
  sim::tomHz = f;
  sim::tons.push_back(f);
}
void noTone(uint8_t) { sim::tomHz = 0; }
bool ledcAttach(uint8_t, uint32_t, uint8_t) { return true; }
bool ledcWrite(uint8_t pin, uint32_t duty) {
  sim::pwm[pin] = duty;
  return true;
}

extern "C" bool getLocalTime(struct tm*, uint32_t) { return false; }
extern "C" void configTime(long, int, const char*, const char*, const char*) {}

int HardwareSerial::printf(const char* fmt, ...) {
  char buf[1024];
  va_list ap;
  va_start(ap, fmt);
  const int n = vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  sim::serial += buf;
  if (sim::serialVisivel) std::cout << buf;
  return n;
}
size_t HardwareSerial::print(const char* s) { return static_cast<size_t>(printf("%s", s)); }
size_t HardwareSerial::println(const char* s) { return static_cast<size_t>(printf("%s\n", s)); }

// ---------- FreeRTOS ----------
SemaphoreHandle_t xSemaphoreCreateMutex() { return reinterpret_cast<void*>(1); }
BaseType_t xSemaphoreTake(SemaphoreHandle_t, TickType_t) { return pdTRUE; }
BaseType_t xSemaphoreGive(SemaphoreHandle_t) { return pdTRUE; }
QueueHandle_t xQueueCreate(uint32_t, uint32_t) { return new std::deque<uint8_t>(); }
BaseType_t xQueueSend(QueueHandle_t q, const void* item, TickType_t) {
  static_cast<std::deque<uint8_t>*>(q)->push_back(*static_cast<const uint8_t*>(item));
  return pdTRUE;
}
BaseType_t xQueueReceive(QueueHandle_t q, void* item, TickType_t) {
  auto* d = static_cast<std::deque<uint8_t>*>(q);
  if (d->empty()) return pdFALSE;
  *static_cast<uint8_t*>(item) = d->front();
  d->pop_front();
  return pdTRUE;
}
BaseType_t xTaskCreatePinnedToCore(TaskFunction_t, const char*, uint32_t, void*, unsigned, TaskHandle_t*, int) {
  return pdTRUE;  // a tarefa de rede não roda na simulação
}

// ---------- WebServer ----------
WebServer::WebServer(int) { sim::servidorAtual = this; }

void WebServer::on(const String& uri, HTTPMethod metodo, THandlerFunction fn) {
  rotas_.push_back({uri.std(), metodo, fn});
}

String WebServer::arg(const String& nome) const {
  auto it = args_.find(nome.std());
  return it == args_.end() ? String("") : String(it->second);
}

void WebServer::send(int code, const char* content_type, const String& content) {
  resposta_.codigo = code;
  resposta_.tipo = content_type ? content_type : "";
  resposta_.corpo += content.std();
}

sim::Resposta WebServer::chamar(const std::string& metodo, const std::string& caminho,
                                const std::map<std::string, std::string>& args) {
  args_ = args;
  resposta_ = sim::Resposta();
  const HTTPMethod m = metodo == "POST" ? HTTP_POST : HTTP_GET;
  for (const Rota& r : rotas_) {
    if (r.uri == caminho && r.metodo == m) {
      r.fn();
      return resposta_;
    }
  }
  if (naoEncontrado_) naoEncontrado_();
  return resposta_;
}
