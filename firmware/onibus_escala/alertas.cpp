#include "alertas.h"

#include <Arduino.h>

namespace {
using Nota = Alertas::Nota;
constexpr Nota INICIO[] = {{660, 90}, {0, 40}, {990, 120}};
constexpr Nota ATENCAO[] = {{900, 120}};
constexpr Nota PARADA[] = {{1200, 120}, {0, 80}, {1200, 120}};
constexpr Nota FREIO[] = {{1800, 80}, {0, 50}, {1800, 80}, {0, 50}, {1800, 80}};
constexpr Nota BUZINA[] = {{440, 400}};
constexpr Nota RESPOSTA[] = {{990, 80}, {0, 40}, {660, 120}};
constexpr Nota MODO[] = {{1300, 60}};

template <size_t N>
constexpr uint8_t tamanho(const Nota (&)[N]) { return static_cast<uint8_t>(N); }
}  // namespace

void Alertas::tocar(Som s, uint32_t agoraMs) {
  switch (s) {
    case Som::INICIO: notas_ = INICIO; total_ = tamanho(INICIO); break;
    case Som::ATENCAO: notas_ = ATENCAO; total_ = tamanho(ATENCAO); break;
    case Som::PARADA: notas_ = PARADA; total_ = tamanho(PARADA); break;
    case Som::FREIO: notas_ = FREIO; total_ = tamanho(FREIO); break;
    case Som::BUZINA: notas_ = BUZINA; total_ = tamanho(BUZINA); break;
    case Som::RESPOSTA: notas_ = RESPOSTA; total_ = tamanho(RESPOSTA); break;
    case Som::MODO: notas_ = MODO; total_ = tamanho(MODO); break;
  }
  atual_ = 0;
  iniciarNota(agoraMs);
}

void Alertas::iniciarNota(uint32_t agoraMs) {
  const Nota& n = notas_[atual_];
  if (n.freqHz > 0) tone(pino_, n.freqHz);
  else noTone(pino_);
  fimNotaMs_ = agoraMs + n.duracaoMs;
}

void Alertas::atualizar(uint32_t agoraMs) {
  if (!notas_ || static_cast<int32_t>(agoraMs - fimNotaMs_) < 0) return;
  if (++atual_ >= total_) {
    noTone(pino_);
    notas_ = nullptr;
    return;
  }
  iniciarNota(agoraMs);
}
