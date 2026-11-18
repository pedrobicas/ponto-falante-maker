#pragma once

#include "WiFi.h"

class WiFiClientSecure : public WiFiClient {
 public:
  void setCACert(const char* raiz) { ca = raiz; }
  const char* ca = nullptr;
};
