// Estado do "mundo simulado" usado pelos stubs do Arduino nos testes no PC.
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace sim {

extern uint32_t agoraMs;          // relógio (millis)
extern float distanciaCm;         // o que o HC-SR04 "vê"; < 0 = sem eco
extern int ldr;                   // leitura do ADC do LDR (0..4095)
extern bool botaoApertado;        // botão BOOT (GPIO 0)
extern int nivel[64];             // digitalWrite por pino
extern uint32_t pwm[64];          // ledcWrite por pino
extern unsigned tomHz;            // frequência atual do buzzer (0 = mudo)
extern std::vector<unsigned> tons;  // histórico de frequências tocadas
extern bool serialVisivel;        // imprime o Serial no terminal
extern std::string serial;        // tudo o que foi impresso
extern std::vector<std::string> tela;  // textos desenhados no último display()

struct Resposta {
  int codigo = 0;
  std::string tipo;
  std::string corpo;
};

// Chama uma rota registrada no WebServer, como se viesse do celular.
Resposta requisicao(const std::string& metodo, const std::string& caminho,
                    const std::map<std::string, std::string>& args = {});

}  // namespace sim
