// Buzzer sem bloquear o loop: cada som é uma sequência de notas tocada aos poucos em atualizar().
#pragma once

#include <stdint.h>

enum class Som : uint8_t { INICIO, ATENCAO, PARADA, FREIO, BUZINA, RESPOSTA, MODO };

class Alertas {
 public:
  void iniciar(uint8_t pino) { pino_ = pino; }
  void tocar(Som s, uint32_t agoraMs);
  void atualizar(uint32_t agoraMs);
  bool tocando() const { return notas_ != nullptr; }

  struct Nota {
    uint16_t freqHz;  // 0 = silêncio
    uint16_t duracaoMs;
  };

 private:
  void iniciarNota(uint32_t agoraMs);

  uint8_t pino_ = 255;
  const Nota* notas_ = nullptr;
  uint8_t total_ = 0;
  uint8_t atual_ = 0;
  uint32_t fimNotaMs_ = 0;
};
