#pragma once

#include "Arduino.h"

typedef enum { WIFI_OFF = 0, WIFI_STA = 1, WIFI_AP = 2, WIFI_AP_STA = 3 } wifi_mode_t;
typedef enum { WL_IDLE_STATUS = 0, WL_CONNECTED = 3, WL_DISCONNECTED = 6 } wl_status_t;

class IPAddress {
 public:
  explicit IPAddress(const char* s = "0.0.0.0") : s_(s) {}
  String toString() const { return String(s_); }

 private:
  const char* s_;
};

class WiFiClass {
 public:
  bool mode(wifi_mode_t m) { modo = m; return true; }
  bool setSleep(bool) { return true; }
  bool softAP(const char* ssid, const char* senha) { redeAp = ssid; (void)senha; return true; }
  IPAddress softAPIP() { return IPAddress("192.168.4.1"); }
  wl_status_t begin(const char* ssid, const char* senha) { (void)ssid; (void)senha; return WL_DISCONNECTED; }
  wl_status_t status() { return WL_DISCONNECTED; }
  IPAddress localIP() { return IPAddress(); }

  wifi_mode_t modo = WIFI_OFF;
  const char* redeAp = "";
};
extern WiFiClass WiFi;

class WiFiClient {
 public:
  virtual ~WiFiClient() = default;
};
