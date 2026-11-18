// Ponte H L298N com controle diferencial (lados pareados).
#pragma once

#include <stdint.h>

enum class Direcao : uint8_t { PARADO, FRENTE, RE, ESQUERDA, DIREITA };

class Motores {
 public:
  void iniciar();
  void mover(Direcao d, int velocidade);
  void parar() { mover(Direcao::PARADO, 0); }

  Direcao direcao() const { return direcao_; }
  int velocidade() const { return velocidade_; }
  static const char* nome(Direcao d);

 private:
  void pinos(bool in1, bool in2, bool in3, bool in4);
  Direcao direcao_ = Direcao::PARADO;
  int velocidade_ = 0;
};
