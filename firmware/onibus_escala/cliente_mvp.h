// Integração com o MVP do Ponto Falante pelo contrato de dispositivos:
//   POST /api/integracoes/dispositivos/eventos  (cabeçalho X-Device-Key)
// Roda numa tarefa separada (núcleo 0) para o HTTPS nunca travar o loop dos motores e do sensor.
#pragma once

#include <stdint.h>

enum class TipoEvento : uint8_t { HEARTBEAT, BOTAO_PRESSIONADO };

struct StatusMvp {
  bool configurado = false;     // tem rede e chave
  bool wifiConectado = false;
  bool horaSincronizada = false;
  bool enviando = false;
  int ultimoCodigo = 0;         // HTTP; negativo = erro de conexão
  char ultimoTipo[20] = "";
  uint32_t ultimoEnvioMs = 0;
  uint32_t enviados = 0;
  char frase[320] = "";         // frase_falada da última consulta
  uint32_t versaoFrase = 0;     // muda a cada frase nova
  char ip[16] = "";
};

class ClienteMvp {
 public:
  void iniciar();
  bool enfileirar(TipoEvento tipo);
  StatusMvp status();  // cópia protegida por mutex
  const char* url() const;
  const char* dispositivoId() const;
  long ponto() const;
};

extern ClienteMvp mvp;
