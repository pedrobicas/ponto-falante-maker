#include "controle_luz.h"

bool ControleLuz::atualizar(int leituraAdc) {
  const int leitura = invertido_ ? 4095 - leituraAdc : leituraAdc;
  suave_ = suave_ < 0 ? leitura : (1.0f - alfa_) * suave_ + alfa_ * leitura;

  if (!escuro_ && suave_ < ligaAbaixo_) escuro_ = true;
  else if (escuro_ && suave_ > desligaAcima_) escuro_ = false;
  return aceso();
}

bool ControleLuz::aceso() const {
  switch (modo_) {
    case ModoLetreiro::LIGADO: return true;
    case ModoLetreiro::DESLIGADO: return false;
    default: return escuro_;
  }
}

const char* ControleLuz::nome(ModoLetreiro m) {
  switch (m) {
    case ModoLetreiro::LIGADO: return "ligado";
    case ModoLetreiro::DESLIGADO: return "desligado";
    default: return "automatico";
  }
}
