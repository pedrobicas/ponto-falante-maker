#include "cliente_mvp.h"

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <string.h>
#include <time.h>

#include "certificados.h"
#include "config.h"
#include "credenciais_ativas.h"
#include "texto.h"

ClienteMvp mvp;

namespace {

constexpr const char* CAMINHO_EVENTOS = "/api/integracoes/dispositivos/eventos";

QueueHandle_t fila = nullptr;
SemaphoreHandle_t trava = nullptr;
StatusMvp estado;
bool horaConfigurada = false;

const char* nomeTipo(TipoEvento t) { return t == TipoEvento::BOTAO_PRESSIONADO ? "BOTAO_PRESSIONADO" : "HEARTBEAT"; }

template <typename F>
void comTrava(F f) {
  xSemaphoreTake(trava, portMAX_DELAY);
  f();
  xSemaphoreGive(trava);
}

bool horaAtual(char* saida, size_t tamanho) {
  struct tm agora;
  if (!getLocalTime(&agora, 10) || agora.tm_year < 2024 - 1900) return false;
  strftime(saida, tamanho, "%Y-%m-%dT%H:%M:%S-03:00", &agora);
  return true;
}

void enviar(TipoEvento tipo) {
  char quando[32];
  if (!horaAtual(quando, sizeof(quando))) {
    Serial.println("[MVP] relógio ainda não sincronizado; evento descartado");
    comTrava([&] { estado.ultimoCodigo = -10; });
    return;
  }

  char corpo[256];
  if (!texto::corpoEvento(MVP_DISPOSITIVO_ID, static_cast<long>(MVP_PONTO), nomeTipo(tipo), quando, corpo,
                          sizeof(corpo))) {
    Serial.println("[MVP] MVP_DISPOSITIVO_ID longo demais");
    return;
  }

  comTrava([&] { estado.enviando = true; });
  String url = String(MVP_URL) + CAMINHO_EVENTOS;
  HTTPClient http;
  WiFiClientSecure seguro;
  WiFiClient simples;
  bool aberto;
  if (url.startsWith("https://")) {
    seguro.setCACert(CERTIFICADOS_RAIZ);
    aberto = http.begin(seguro, url);
  } else {
    aberto = http.begin(simples, url);  // só para testes na rede local
  }

  int codigo = -1;
  String resposta;
  if (aberto) {
    http.setConnectTimeout(10000);
    http.setTimeout(static_cast<uint16_t>(config::HTTP_TIMEOUT_MS));
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Device-Key", MVP_CHAVE_DISPOSITIVO);
    codigo = http.POST(String(corpo));
    if (codigo > 0) resposta = http.getString();
    http.end();
  }
  Serial.printf("[MVP] %s -> HTTP %d\n", nomeTipo(tipo), codigo);

  char frase[sizeof(estado.frase)];
  const bool temFrase = codigo == 202 && texto::campoJson(resposta.c_str(), "frase_falada", frase, sizeof(frase));
  if (temFrase) Serial.printf("[MVP] frase: %s\n", frase);

  comTrava([&] {
    estado.enviando = false;
    estado.ultimoCodigo = codigo;
    strncpy(estado.ultimoTipo, nomeTipo(tipo), sizeof(estado.ultimoTipo) - 1);
    estado.ultimoEnvioMs = millis();
    if (codigo == 202) estado.enviados++;
    if (tipo == TipoEvento::BOTAO_PRESSIONADO) {
      const char* msg = temFrase         ? frase
                          : codigo == 401 ? "Chave do dispositivo recusada pelo MVP."
                          : codigo == 503 ? "MVP sem chave de dispositivo configurada."
                                          : "Não consegui falar com o MVP agora.";
      strncpy(estado.frase, msg, sizeof(estado.frase) - 1);
      estado.versaoFrase++;
    }
  });
}

void tarefa(void*) {
  uint32_t ultimoHeartbeat = 0;
  bool primeiro = true;
  for (;;) {
    const bool conectado = WiFi.status() == WL_CONNECTED;
    if (conectado && !horaConfigurada) {
      configTime(config::FUSO_S, 0, "pool.ntp.org", "time.google.com");
      horaConfigurada = true;
    }
    char quando[32];
    const bool hora = conectado && horaAtual(quando, sizeof(quando));
    comTrava([&] {
      estado.wifiConectado = conectado;
      estado.horaSincronizada = hora;
      if (conectado) strncpy(estado.ip, WiFi.localIP().toString().c_str(), sizeof(estado.ip) - 1);
      else estado.ip[0] = '\0';
    });

    TipoEvento ev;
    if (xQueueReceive(fila, &ev, pdMS_TO_TICKS(1000)) == pdTRUE) {
      if (conectado) enviar(ev);
      else comTrava([&] {
        estado.ultimoCodigo = -20;
        if (ev == TipoEvento::BOTAO_PRESSIONADO) {
          strncpy(estado.frase, "Sem internet: o carrinho não está conectado à rede do MVP.", sizeof(estado.frase) - 1);
          estado.versaoFrase++;
        }
      });
    } else if (hora && (primeiro || millis() - ultimoHeartbeat >= config::HEARTBEAT_MS)) {
      ultimoHeartbeat = millis();
      primeiro = false;
      enviar(TipoEvento::HEARTBEAT);
    }
  }
}

}  // namespace

void ClienteMvp::iniciar() {
  trava = xSemaphoreCreateMutex();
  estado.configurado = strlen(WIFI_REDE) > 0 && strlen(MVP_CHAVE_DISPOSITIVO) > 0;
  if (!estado.configurado) {
    Serial.println("[MVP] sem rede ou sem chave em credenciais.h: integração desligada");
    return;
  }
  fila = xQueueCreate(4, sizeof(TipoEvento));
  // TLS precisa de pilha grande; núcleo 0 fica com a rede, o loop do Arduino segue no núcleo 1.
  xTaskCreatePinnedToCore(tarefa, "mvp", 16384, nullptr, 1, nullptr, 0);
  Serial.printf("[MVP] integração ligada: %s, ponto %ld\n", MVP_URL, static_cast<long>(MVP_PONTO));
}

bool ClienteMvp::enfileirar(TipoEvento tipo) {
  if (!fila) {
    comTrava([&] {
      if (tipo == TipoEvento::BOTAO_PRESSIONADO) {
        strncpy(estado.frase, "Integração com o MVP desligada: preencha credenciais.h.", sizeof(estado.frase) - 1);
        estado.versaoFrase++;
      }
    });
    return false;
  }
  return xQueueSend(fila, &tipo, 0) == pdTRUE;
}

StatusMvp ClienteMvp::status() {
  StatusMvp copia;
  comTrava([&] { copia = estado; });
  return copia;
}

const char* ClienteMvp::url() const { return MVP_URL; }
const char* ClienteMvp::dispositivoId() const { return MVP_DISPOSITIVO_ID; }
long ClienteMvp::ponto() const { return static_cast<long>(MVP_PONTO); }
