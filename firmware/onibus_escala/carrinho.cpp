#include "carrinho.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "cliente_mvp.h"
#include "texto.h"

using namespace config;

Carrinho carrinho;

namespace {
const ParametrosPiloto PARAMETROS = {VEL_CRUZEIRO,  VEL_REDUZIDA,      CONFIANCA_MIN, DIST_PARADA_CM,
                                     DIST_FREIO_CM, LIVRES_PARA_SEGUIR, PARADA_MIN_MS};

float medirDistanciaCm() {
  digitalWrite(PINO_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PINO_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PINO_TRIG, LOW);
  const unsigned long us = pulseIn(PINO_ECHO, HIGH, ECO_TIMEOUT_US);
  if (us == 0) return -1;  // sem eco
  return us * 0.0343f / 2.0f;
}

const char* nomeModo(Modo m) { return m == Modo::AUTONOMO ? "autonomo" : "manual"; }
}  // namespace

Carrinho::Carrinho() : piloto_(PARAMETROS), luz_(LUZ_LIGA_ABAIXO, LUZ_DESLIGA_ACIMA, LUZ_SUAVIZACAO, LDR_INVERTIDO) {}

void Carrinho::iniciar() {
  pinMode(PINO_TRIG, OUTPUT);
  pinMode(PINO_ECHO, INPUT);
  pinMode(PINO_LETREIRO, OUTPUT);
  digitalWrite(PINO_LETREIRO, LOW);
  pinMode(PINO_LDR, INPUT);
  pinMode(PINO_BOTAO, INPUT_PULLUP);
  pinMode(PINO_BUZZER, OUTPUT);

  motores_.iniciar();
  alertas_.iniciar(PINO_BUZZER);
  Serial.println(tela_.iniciar(OLED_ENDERECO) ? "[BOOT] display OK" : "[BOOT] display não encontrado (segue sem)");

  const uint32_t agora = millis();
  piloto_.reiniciar(agora);
  proximaLeitura_ = proximaLuz_ = proximaTela_ = agora;
  alertas_.tocar(Som::INICIO, agora);
}

void Carrinho::atualizar() {
  const uint32_t agora = millis();
  alertas_.atualizar(agora);
  lerLuz(agora);
  lerDistancia(agora);
  lerBotao(agora);

  // Homem-morto do modo manual: sem comando recente do painel, para.
  if (modo_ == Modo::MANUAL && motores_.direcao() != Direcao::PARADO && agora - ultimoManual_ > MANUAL_TIMEOUT_MS) {
    motores_.parar();
  }

  if (static_cast<int32_t>(agora - proximaMvp_) >= 0) {
    proximaMvp_ = agora + 100;
    const StatusMvp s = mvp.status();
    if (s.versaoFrase != versaoFrase_) {
      versaoFrase_ = s.versaoFrase;
      strncpy(frase_, s.frase, sizeof(frase_) - 1);
      fraseAte_ = agora + 10000;
      alertas_.tocar(Som::RESPOSTA, agora);
    }
  }
  atualizarTela(agora);
}

// ---------- sensores ----------

void Carrinho::lerLuz(uint32_t agora) {
  if (static_cast<int32_t>(agora - proximaLuz_) < 0) return;
  proximaLuz_ = agora + PERIODO_LUZ_MS;
  const bool antes = luz_.aceso();
  const bool aceso = luz_.atualizar(analogRead(PINO_LDR));
  digitalWrite(PINO_LETREIRO, aceso ? HIGH : LOW);
  if (aceso != antes) {
    Serial.printf("[LETREIRO] %s (luz %d, modo %s)\n", aceso ? "aceso" : "apagado", luz_.nivel(),
                  ControleLuz::nome(luz_.modo()));
  }
}

void Carrinho::lerDistancia(uint32_t agora) {
  if (static_cast<int32_t>(agora - proximaLeitura_) < 0) return;
  if (agora - proximaLeitura_ > 4 * PERIODO_LEITURA_MS) {
    // O loop ficou parado: a janela tem buracos e não representa mais 1,5 s contínuos.
    ia_.reiniciar();
    proximaLeitura_ = agora;
    Serial.println("[IA] janela descartada (loop atrasou), enchendo de novo");
  }
  proximaLeitura_ += PERIODO_LEITURA_MS;

  distancia_ = filtro_.aplicar(medirDistanciaCm());
  ia_.adicionar(distancia_);
  contador_++;

  if (gravando_) {
    if (nColeta_ < COLETA_MAX_AMOSTRAS) {
      coleta_[nColeta_] = distancia_;
      Serial.printf("%d,%.1f\n", nColeta_ * static_cast<int>(PERIODO_LEITURA_MS), distancia_);
      nColeta_++;
    } else {
      gravar(false);
    }
  }

  const bool inferir = ia_.pronto() && contador_ % INFERIR_A_CADA == 0;
  const int classeAntes = resultado_.classe;
  if (inferir) resultado_ = ia_.classificar();
  else if (!ia_.pronto()) resultado_ = Resultado();

  if (modo_ == Modo::AUTONOMO) {
    const Comando c = inferir ? piloto_.atualizar(distancia_, resultado_.classe, resultado_.confianca, agora)
                              : piloto_.atualizar(distancia_, SEM_CLASSE, 0, agora);
    motores_.mover(c.velocidade > 0 ? Direcao::FRENTE : Direcao::PARADO, c.velocidade);
    aplicarAviso(c.aviso, agora);
  } else if (motores_.direcao() == Direcao::FRENTE && distancia_ <= DIST_FREIO_CM) {
    // No manual também não deixa bater de frente.
    motores_.parar();
    aplicarAviso(Aviso::FREIO, agora);
  }

  if (inferir) registrarIa(agora, resultado_.classe != classeAntes || piloto_.estado() != estadoAnterior_);
  estadoAnterior_ = piloto_.estado();
}

void Carrinho::registrarIa(uint32_t agora, bool mudou) {
  if (gravando_ || (!mudou && agora - ultimoLogIa_ < 1000)) return;
  ultimoLogIa_ = agora;
  Serial.printf("[IA] livre=%.2f aproximando=%.2f obstaculo=%.2f | dist=%.1f cm -> %s", resultado_.prob[LIVRE],
                resultado_.prob[APROXIMANDO], resultado_.prob[OBSTACULO], distancia_,
                Classificador::nome(resultado_.classe));
  if (modo_ == Modo::AUTONOMO) Serial.printf(" | %s", Piloto::nome(piloto_.estado()));
  Serial.println();
}

void Carrinho::lerBotao(uint32_t agora) {
  const bool apertado = digitalRead(PINO_BOTAO) == LOW;
  if (apertado && !botaoAnterior_) {
    botaoDesde_ = agora;
    botaoLongoTratado_ = false;
  }
  // Segurar 1 s: alterna manual/autônomo. Toque curto: pede ao MVP os próximos ônibus do ponto.
  if (apertado && !botaoLongoTratado_ && agora - botaoDesde_ >= 1000) {
    botaoLongoTratado_ = true;
    definirModo(modo_ == Modo::AUTONOMO ? Modo::MANUAL : Modo::AUTONOMO);
  }
  if (!apertado && botaoAnterior_ && !botaoLongoTratado_ && agora - botaoDesde_ >= 30) consultarPonto();
  botaoAnterior_ = apertado;
}

void Carrinho::aplicarAviso(Aviso a, uint32_t agora) {
  switch (a) {
    case Aviso::ATENCAO: alertas_.tocar(Som::ATENCAO, agora); break;
    case Aviso::PARADA: alertas_.tocar(Som::PARADA, agora); break;
    case Aviso::FREIO: alertas_.tocar(Som::FREIO, agora); break;
    default: break;
  }
}

// ---------- comandos ----------

void Carrinho::comandoManual(Direcao d) {
  if (modo_ != Modo::MANUAL) definirModo(Modo::MANUAL);
  ultimoManual_ = millis();
  if (d == Direcao::FRENTE && distancia_ <= DIST_FREIO_CM) d = Direcao::PARADO;
  motores_.mover(d, d == Direcao::PARADO ? 0 : VEL_MANUAL);
}

void Carrinho::definirModo(Modo m) {
  if (m == modo_) return;
  modo_ = m;
  motores_.parar();
  piloto_.reiniciar(millis());
  estadoAnterior_ = Estado::AGUARDANDO;
  alertas_.tocar(Som::MODO, millis());
  Serial.printf("[MODO] %s\n", nomeModo(m));
}

void Carrinho::definirLetreiro(ModoLetreiro m) {
  luz_.definirModo(m);
  proximaLuz_ = millis();  // aplica já
  Serial.printf("[LETREIRO] modo %s\n", ControleLuz::nome(m));
}

void Carrinho::buzina() { alertas_.tocar(Som::BUZINA, millis()); }

void Carrinho::consultarPonto() {
  Serial.printf("[MVP] consulta do ponto %ld pedida\n", mvp.ponto());
  if (!mvp.enfileirar(TipoEvento::BOTAO_PRESSIONADO)) Serial.println("[MVP] consulta não enfileirada");
}

void Carrinho::gravar(bool ligar) {
  if (ligar == gravando_) return;
  gravando_ = ligar;
  if (ligar) {
    nColeta_ = 0;
    Serial.println("timestamp,dist_cm");
  } else {
    Serial.printf("[COLETA] %d amostras gravadas\n", nColeta_);
  }
}

// ---------- saída ----------

void Carrinho::atualizarTela(uint32_t agora) {
  if (static_cast<int32_t>(agora - proximaTela_) < 0) return;
  proximaTela_ = agora + 250;

  if (static_cast<int32_t>(fraseAte_ - agora) > 0) {
    tela_.desenharTexto("PROXIMOS", frase_);
    return;
  }

  char l1[22], l2[22], l3[22], l4[22];
  snprintf(l1, sizeof(l1), "dist %5.1f cm", distancia_);
  if (resultado_.classe == SEM_CLASSE) snprintf(l2, sizeof(l2), "IA enchendo janela");
  else snprintf(l2, sizeof(l2), "IA %s %d%%", Classificador::nome(resultado_.classe),
                static_cast<int>(resultado_.confianca * 100 + 0.5f));
  snprintf(l3, sizeof(l3), "luz %4d letreiro %s", luz_.nivel(), luz_.aceso() ? "ON" : "--");
  const StatusMvp s = mvp.status();
  if (s.configurado && s.wifiConectado) snprintf(l4, sizeof(l4), "MVP HTTP %d", s.ultimoCodigo);
  else snprintf(l4, sizeof(l4), "painel 192.168.4.1");

  const char* linhas[] = {l1, l2, l3, l4};
  const char* titulo = gravando_ ? "GRAVANDO" : modo_ == Modo::MANUAL ? "MANUAL" : Piloto::nome(piloto_.estado());
  tela_.desenhar(titulo, linhas, 4);
}

size_t Carrinho::estadoJson(char* buf, size_t tamanho) {
  const StatusMvp s = mvp.status();
  char frase[sizeof(s.frase) + 64];
  if (!texto::escaparJson(s.frase, frase, sizeof(frase))) strcpy(frase, "\"\"");
  const int haS = s.ultimoEnvioMs ? static_cast<int>((millis() - s.ultimoEnvioMs) / 1000) : -1;

  const int n = snprintf(
      buf, tamanho,
      "{\"modo\":\"%s\",\"estado\":\"%s\",\"distancia\":%.1f,\"janela_pronta\":%s,"
      "\"classe\":\"%s\",\"confianca\":%.3f,\"prob\":[%.3f,%.3f,%.3f],"
      "\"motor\":\"%s\",\"velocidade\":%d,"
      "\"luz\":%d,\"letreiro\":%s,\"letreiro_modo\":\"%s\","
      "\"gravando\":%s,\"amostras\":%d,\"amostras_max\":%d,"
      "\"mvp\":{\"configurado\":%s,\"wifi\":%s,\"hora\":%s,\"enviando\":%s,\"codigo\":%d,\"tipo\":\"%s\","
      "\"ha_s\":%d,\"enviados\":%lu,\"frase\":%s,\"versao\":%lu,\"ip\":\"%s\",\"ponto\":%ld,\"url\":\"%s\"},"
      "\"uptime_s\":%lu}",
      nomeModo(modo_), modo_ == Modo::MANUAL ? "MANUAL" : Piloto::nome(piloto_.estado()), distancia_,
      ia_.pronto() ? "true" : "false", Classificador::nome(resultado_.classe), resultado_.confianca,
      resultado_.prob[0], resultado_.prob[1], resultado_.prob[2], Motores::nome(motores_.direcao()),
      motores_.velocidade(), luz_.nivel(), luz_.aceso() ? "true" : "false", ControleLuz::nome(luz_.modo()),
      gravando_ ? "true" : "false", nColeta_, COLETA_MAX_AMOSTRAS, s.configurado ? "true" : "false",
      s.wifiConectado ? "true" : "false", s.horaSincronizada ? "true" : "false", s.enviando ? "true" : "false",
      s.ultimoCodigo, s.ultimoTipo, haS, static_cast<unsigned long>(s.enviados), frase,
      static_cast<unsigned long>(s.versaoFrase), s.ip, mvp.ponto(), mvp.url(),
      static_cast<unsigned long>(millis() / 1000));
  return n > 0 && static_cast<size_t>(n) < tamanho ? static_cast<size_t>(n) : 0;
}
