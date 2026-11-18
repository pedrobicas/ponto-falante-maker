// Assinaturas de libraries/WebServer/src/WebServer.h (esp32 3.x), com um modo de simulação para os testes.
#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "Arduino.h"

typedef enum { HTTP_DELETE = 0, HTTP_GET = 1, HTTP_HEAD = 2, HTTP_POST = 3, HTTP_PUT = 4 } HTTPMethod;
#define CONTENT_LENGTH_UNKNOWN ((size_t)-1)

class WebServer {
 public:
  typedef std::function<void(void)> THandlerFunction;

  explicit WebServer(int porta = 80);
  void on(const String& uri, HTTPMethod metodo, THandlerFunction fn);
  void onNotFound(THandlerFunction fn) { naoEncontrado_ = fn; }
  void begin() { iniciado_ = true; }
  void handleClient() {}

  String arg(const String& nome) const;
  void send(int code, const char* content_type = nullptr, const String& content = String(""));
  void send(int code, const char* content_type, const char* content) { send(code, content_type, String(content)); }
  void send_P(int code, const char* content_type, const char* content) { send(code, content_type, String(content)); }
  void sendHeader(const String& nome, const String& valor, bool first = false) { (void)nome; (void)valor; (void)first; }
  void setContentLength(size_t) {}
  void sendContent(const String& c) { resposta_.corpo += c.std(); }
  void sendContent(const char* c, size_t n) { resposta_.corpo.append(c, n); }

  // simulação
  sim::Resposta chamar(const std::string& metodo, const std::string& caminho,
                       const std::map<std::string, std::string>& args);

 private:
  struct Rota {
    std::string uri;
    HTTPMethod metodo;
    THandlerFunction fn;
  };
  std::vector<Rota> rotas_;
  THandlerFunction naoEncontrado_;
  std::map<std::string, std::string> args_;
  sim::Resposta resposta_;
  bool iniciado_ = false;
};
