#include "filtro_distancia.h"

namespace {
float mediana3(float a, float b, float c) {
  if (a > b) { float t = a; a = b; b = t; }
  if (b > c) { float t = b; b = c; c = t; }
  if (a > b) { float t = a; a = b; b = t; }
  return b;
}
}  // namespace

float FiltroDistancia::aplicar(float bruta) {
  float d = bruta;
  if (d < 0 || d >= SEM_ECO_CM) {
    d = SEM_ECO_CM;
  } else if (d < MINIMO_CM) {
    d = ultima_;
  }
  ultima_ = d;
  historico_[0] = historico_[1];
  historico_[1] = historico_[2];
  historico_[2] = d;
  return mediana3(historico_[0], historico_[1], historico_[2]);
}

void FiltroDistancia::reiniciar() {
  ultima_ = SEM_ECO_CM;
  for (float& h : historico_) h = SEM_ECO_CM;
}
