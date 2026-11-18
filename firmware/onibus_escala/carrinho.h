// Junta sensores, IA, piloto, atuadores e display. O painel e o botão BOOT chamam os comandos abaixo.
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "alertas.h"
#include "classificador.h"
#include "config.h"
#include "controle_luz.h"
#include "filtro_distancia.h"
#include "motores.h"
#include "piloto.h"
#include "tela.h"

enum class Modo : uint8_t { MANUAL, AUTONOMO };

class Carrinho {
 public:
  Carrinho();
  void iniciar();
  void atualizar();

  // Comandos
  void comandoManual(Direcao d);
  void definirModo(Modo m);
  void definirLetreiro(ModoLetreiro m);
  void buzina();
  void consultarPonto();
  void gravar(bool ligar);

  // Leitura
  Modo modo() const { return modo_; }
  size_t estadoJson(char* buf, size_t tamanho);
  int amostrasGravadas() const { return nColeta_; }
  float amostra(int i) const { return coleta_[i]; }

 private:
  void lerDistancia(uint32_t agora);
  void lerLuz(uint32_t agora);
  void lerBotao(uint32_t agora);
  void aplicarAviso(Aviso a, uint32_t agora);
  void atualizarTela(uint32_t agora);
  void registrarIa(uint32_t agora, bool mudou);

  Modo modo_ = Modo::MANUAL;
  Motores motores_;
  FiltroDistancia filtro_;
  Classificador ia_;
  Piloto piloto_;
  ControleLuz luz_;
  Alertas alertas_;
  Tela tela_;

  float distancia_ = FiltroDistancia::SEM_ECO_CM;
  Resultado resultado_;
  Estado estadoAnterior_ = Estado::AGUARDANDO;
  int contador_ = 0;

  uint32_t proximaLeitura_ = 0;
  uint32_t proximaLuz_ = 0;
  uint32_t proximaTela_ = 0;
  uint32_t ultimoLogIa_ = 0;
  uint32_t ultimoManual_ = 0;
  uint32_t fraseAte_ = 0;
  uint32_t proximaMvp_ = 0;
  uint32_t versaoFrase_ = 0;
  char frase_[320] = "";

  bool botaoAnterior_ = false;
  bool botaoLongoTratado_ = false;
  uint32_t botaoDesde_ = 0;

  bool gravando_ = false;
  float coleta_[config::COLETA_MAX_AMOSTRAS];
  int nColeta_ = 0;
};

extern Carrinho carrinho;
