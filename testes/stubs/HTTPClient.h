// Assinaturas de libraries/HTTPClient/src/HTTPClient.h (esp32 3.x).
#pragma once

#include "WiFi.h"

class HTTPClient {
 public:
  bool begin(WiFiClient& client, String url) { (void)client; url_ = url; return true; }
  void setConnectTimeout(int32_t) {}
  void setTimeout(uint16_t) {}
  void addHeader(const String& nome, const String& valor, bool first = false, bool replace = true) {
    (void)nome; (void)valor; (void)first; (void)replace;
  }
  int POST(String corpo) { (void)corpo; return -1; }
  String getString() { return String(""); }
  void end() {}

 private:
  String url_;
};
