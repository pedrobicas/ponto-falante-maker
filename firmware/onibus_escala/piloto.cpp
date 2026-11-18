#include "piloto.h"

#include "classificador.h"

void Piloto::reiniciar(uint32_t agoraMs) {
  estado_ = Estado::AGUARDANDO;
  paradoDesde_ = agoraMs;
  livresSeguidos_ = 0;
}

Estado Piloto::entrar(Estado novo, uint32_t agoraMs, Aviso* aviso) {
  if (novo == estado_) return estado_;
  const bool parando = novo == Estado::PARADO || novo == Estado::FREIO;
  const bool jaParado = estado_ == Estado::PARADO || estado_ == Estado::FREIO;
  if (parando && !jaParado) paradoDesde_ = agoraMs;

  if (novo == Estado::FREIO) *aviso = Aviso::FREIO;
  else if (novo == Estado::PARADO && !jaParado) *aviso = Aviso::PARADA;
  else if (novo == Estado::REDUZINDO) *aviso = Aviso::ATENCAO;

  estado_ = novo;
  return estado_;
}

Comando Piloto::atualizar(float distanciaCm, int classe, float confianca, uint32_t agoraMs) {
  Comando c;
  const bool inferiu = classe != SEM_CLASSE;
  const bool confiavel = inferiu && confianca >= p_.confiancaMin;
  const bool parado = estado_ == Estado::PARADO || estado_ == Estado::FREIO || estado_ == Estado::AGUARDANDO;

  if (inferiu) livresSeguidos_ = (confiavel && classe == LIVRE) ? livresSeguidos_ + 1 : 0;

  // 1) Regras fixas: valem a cada leitura, mesmo que a IA erre ou ainda não tenha janela cheia.
  if (distanciaCm <= p_.distFreioCm) {
    entrar(Estado::FREIO, agoraMs, &c.aviso);
  } else if (distanciaCm <= p_.distParadaCm) {
    entrar(Estado::PARADO, agoraMs, &c.aviso);
  } else if (estado_ == Estado::FREIO) {
    // Saiu da zona de freio: fica parado até a IA confirmar caminho livre.
    entrar(Estado::PARADO, agoraMs, &c.aviso);
  } else if (confiavel) {
    // 2) Decisão da IA.
    switch (classe) {
      case OBSTACULO:
        entrar(Estado::PARADO, agoraMs, &c.aviso);
        break;
      case APROXIMANDO:
        // Parado, alguém se aproximando do ônibus: continua parado.
        if (!parado) entrar(Estado::REDUZINDO, agoraMs, &c.aviso);
        break;
      case LIVRE: {
        const bool parouDeFato = estado_ == Estado::PARADO || estado_ == Estado::FREIO;
        const bool esperouParado = !parouDeFato || agoraMs - paradoDesde_ >= p_.paradaMinMs;
        if (livresSeguidos_ >= p_.livresParaSeguir && esperouParado) entrar(Estado::SEGUINDO, agoraMs, &c.aviso);
        break;
      }
      default:
        break;
    }
  }
  // 3) Sem inferência confiável: mantém o estado anterior.

  c.estado = estado_;
  switch (estado_) {
    case Estado::SEGUINDO: c.velocidade = p_.velCruzeiro; break;
    case Estado::REDUZINDO: c.velocidade = p_.velReduzida; break;
    default: c.velocidade = 0; break;
  }
  return c;
}

const char* Piloto::nome(Estado e) {
  switch (e) {
    case Estado::AGUARDANDO: return "AGUARDANDO";
    case Estado::SEGUINDO: return "SEGUINDO";
    case Estado::REDUZINDO: return "REDUZINDO";
    case Estado::PARADO: return "PARADO";
    case Estado::FREIO: return "FREIO";
  }
  return "?";
}
