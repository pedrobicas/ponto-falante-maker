// Decisão do modo autônomo: junta a classe da IA com duas regras fixas de distância.
// Sem dependência do Arduino (testado no PC em testes/host).
#pragma once

#include <stdint.h>

enum class Estado : uint8_t { AGUARDANDO, SEGUINDO, REDUZINDO, PARADO, FREIO };

// Som emitido ao entrar num estado (o buzzer faz o papel do aviso sonoro externo do ônibus).
enum class Aviso : uint8_t { NENHUM, ATENCAO, PARADA, FREIO };

struct Comando {
  Estado estado = Estado::AGUARDANDO;
  int velocidade = 0;  // 0 = motores parados
  Aviso aviso = Aviso::NENHUM;
};

struct ParametrosPiloto {
  int velCruzeiro;
  int velReduzida;
  float confiancaMin;
  float distParadaCm;
  float distFreioCm;
  int livresParaSeguir;
  uint32_t paradaMinMs;
};

class Piloto {
 public:
  explicit Piloto(const ParametrosPiloto& p) : p_(p) {}

  void reiniciar(uint32_t agoraMs);

  // Chamado a cada leitura (10 Hz). classe = -1 quando não houve inferência nesta leitura.
  Comando atualizar(float distanciaCm, int classe, float confianca, uint32_t agoraMs);

  Estado estado() const { return estado_; }
  static const char* nome(Estado e);

 private:
  Estado entrar(Estado novo, uint32_t agoraMs, Aviso* aviso);

  ParametrosPiloto p_;
  Estado estado_ = Estado::AGUARDANDO;
  uint32_t paradoDesde_ = 0;
  int livresSeguidos_ = 0;
};
