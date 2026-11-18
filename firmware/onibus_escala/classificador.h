// IA embarcada: janela deslizante de distâncias -> atributos de tendência -> regressão logística.
// Pesos em modelo_aproximacao.h, gerados por ia/treinar.py. Sem dependência do Arduino.
#pragma once

#include "modelo_aproximacao.h"

enum Classe : int { LIVRE = 0, APROXIMANDO = 1, OBSTACULO = 2, SEM_CLASSE = -1 };

struct Resultado {
  int classe = SEM_CLASSE;
  float confianca = 0;
  float prob[modelo::N_CLASSES] = {0, 0, 0};
};

class Classificador {
 public:
  void adicionar(float distanciaCm);
  bool pronto() const { return cheia_; }
  void reiniciar();

  // Atributos da janela atual, na ordem de modelo::MEDIA.
  void atributos(float saida[modelo::N_ATRIBUTOS]) const;
  // Classifica a janela atual. Só chame com pronto() == true.
  Resultado classificar() const;
  // Classifica atributos já calculados (usado no teste de paridade com o Python).
  static Resultado classificarAtributos(const float atrib[modelo::N_ATRIBUTOS]);

  static const char* nome(int classe);

 private:
  float janela_[modelo::N_JANELA] = {};
  int proxima_ = 0;
  bool cheia_ = false;
};
