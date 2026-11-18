#include "motores.h"

#include <Arduino.h>

#include "config.h"

using namespace config;

void Motores::iniciar() {
  pinMode(PINO_IN1, OUTPUT);
  pinMode(PINO_IN2, OUTPUT);
  pinMode(PINO_IN3, OUTPUT);
  pinMode(PINO_IN4, OUTPUT);
  ledcAttach(PINO_ENA, PWM_FREQ_HZ, PWM_BITS);
  ledcAttach(PINO_ENB, PWM_FREQ_HZ, PWM_BITS);
  direcao_ = Direcao::FRENTE;  // força a escrita dos pinos no parar()
  parar();
}

void Motores::pinos(bool in1, bool in2, bool in3, bool in4) {
  digitalWrite(PINO_IN1, in1 ? HIGH : LOW);
  digitalWrite(PINO_IN2, in2 ? HIGH : LOW);
  digitalWrite(PINO_IN3, in3 ? HIGH : LOW);
  digitalWrite(PINO_IN4, in4 ? HIGH : LOW);
}

void Motores::mover(Direcao d, int velocidade) {
  if (velocidade <= 0) d = Direcao::PARADO;
  if (d == Direcao::PARADO) velocidade = 0;
  if (velocidade > 255) velocidade = 255;
  if (d == direcao_ && velocidade == velocidade_) return;

  if (d != direcao_) {
    switch (d) {
      case Direcao::FRENTE: pinos(true, false, true, false); break;
      case Direcao::RE: pinos(false, true, false, true); break;
      case Direcao::ESQUERDA: pinos(false, true, true, false); break;
      case Direcao::DIREITA: pinos(true, false, false, true); break;
      case Direcao::PARADO: pinos(false, false, false, false); break;
    }
  }
  Serial.printf("[MOTOR] %s %d\n", nome(d), velocidade);
  ledcWrite(PINO_ENA, velocidade);
  ledcWrite(PINO_ENB, velocidade);
  direcao_ = d;
  velocidade_ = velocidade;
}

const char* Motores::nome(Direcao d) {
  switch (d) {
    case Direcao::FRENTE: return "frente";
    case Direcao::RE: return "re";
    case Direcao::ESQUERDA: return "esquerda";
    case Direcao::DIREITA: return "direita";
    default: return "parado";
  }
}
