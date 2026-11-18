#include "classificador.h"

#include <math.h>

void Classificador::adicionar(float distanciaCm) {
  janela_[proxima_] = distanciaCm;
  proxima_ = (proxima_ + 1) % modelo::N_JANELA;
  if (proxima_ == 0) cheia_ = true;
}

void Classificador::reiniciar() {
  proxima_ = 0;
  cheia_ = false;
}

// Mesmo cálculo de atributos() em ia/treinar.py.
void Classificador::atributos(float saida[modelo::N_ATRIBUTOS]) const {
  constexpr int n = modelo::N_JANELA;
  float bruto[n];
  float x[n];
  for (int i = 0; i < n; i++) {
    bruto[i] = janela_[(proxima_ + i) % n];  // da mais antiga para a mais nova
    x[i] = bruto[i] < modelo::LIMITE_CM ? bruto[i] : modelo::LIMITE_CM;
  }

  float soma = 0, minimo = x[0];
  int semEco = 0;
  for (int i = 0; i < n; i++) {
    soma += x[i];
    if (x[i] < minimo) minimo = x[i];
    if (bruto[i] >= modelo::SEM_ECO_CM - 0.05f) semEco++;
  }
  const float media = soma / n;

  const float tMedio = (n - 1) * modelo::PERIODO_S / 2.0f;
  float cov = 0, varT = 0, varX = 0;
  for (int i = 0; i < n; i++) {
    const float dt = i * modelo::PERIODO_S - tMedio;
    const float dx = x[i] - media;
    cov += dt * dx;
    varT += dt * dt;
    varX += dx * dx;
  }

  saida[0] = x[n - 1];                                                 // ultimo
  saida[1] = media;                                                    // media
  saida[2] = minimo;                                                   // minimo
  saida[3] = sqrtf(varX / n);                                          // desvio
  saida[4] = cov / varT;                                               // inclinacao (cm/s)
  saida[5] = (x[0] + x[1] + x[2]) / 3.0f - (x[n - 3] + x[n - 2] + x[n - 1]) / 3.0f;  // queda
  saida[6] = static_cast<float>(semEco) / n;                           // sem_eco
}

Resultado Classificador::classificarAtributos(const float atrib[modelo::N_ATRIBUTOS]) {
  float logit[modelo::N_CLASSES];
  float maior = -INFINITY;
  for (int c = 0; c < modelo::N_CLASSES; c++) {
    float z = modelo::VIES[c];
    for (int j = 0; j < modelo::N_ATRIBUTOS; j++) {
      z += modelo::PESOS[c][j] * (atrib[j] - modelo::MEDIA[j]) / modelo::ESCALA[j];
    }
    logit[c] = z;
    if (z > maior) maior = z;
  }

  Resultado r;
  float soma = 0;
  for (int c = 0; c < modelo::N_CLASSES; c++) {
    r.prob[c] = expf(logit[c] - maior);  // softmax estável
    soma += r.prob[c];
  }
  for (int c = 0; c < modelo::N_CLASSES; c++) {
    r.prob[c] /= soma;
    if (r.prob[c] > r.confianca) {
      r.confianca = r.prob[c];
      r.classe = c;
    }
  }
  return r;
}

Resultado Classificador::classificar() const {
  float atrib[modelo::N_ATRIBUTOS];
  atributos(atrib);
  return classificarAtributos(atrib);
}

const char* Classificador::nome(int classe) {
  if (classe < 0 || classe >= modelo::N_CLASSES) return "-";
  return modelo::CLASSES[classe];
}
