// Letreiro aceso com pouca luz: média móvel da leitura do LDR + histerese.
// Sem dependência do Arduino (testado no PC em testes/host).
#pragma once

enum class ModoLetreiro : unsigned char { AUTOMATICO, LIGADO, DESLIGADO };

class ControleLuz {
 public:
  ControleLuz(int ligaAbaixo, int desligaAcima, float suavizacao, bool invertido)
      : ligaAbaixo_(ligaAbaixo), desligaAcima_(desligaAcima), alfa_(suavizacao), invertido_(invertido) {}

  // leituraAdc: 0..4095. Devolve se o letreiro deve ficar aceso.
  bool atualizar(int leituraAdc);

  void definirModo(ModoLetreiro m) { modo_ = m; }
  ModoLetreiro modo() const { return modo_; }
  int nivel() const { return suave_ < 0 ? -1 : static_cast<int>(suave_ + 0.5f); }
  bool escuro() const { return escuro_; }
  bool aceso() const;

  static const char* nome(ModoLetreiro m);

 private:
  int ligaAbaixo_;
  int desligaAcima_;
  float alfa_;
  bool invertido_;
  float suave_ = -1;
  bool escuro_ = false;
  ModoLetreiro modo_ = ModoLetreiro::AUTOMATICO;
};
