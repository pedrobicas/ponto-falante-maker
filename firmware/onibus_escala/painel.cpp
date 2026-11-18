#include "painel.h"

#include <Arduino.h>
#include <WebServer.h>

#include "carrinho.h"
#include "painel_html.h"

Painel painel;

namespace {
WebServer servidor(80);

void responderOk() {
  servidor.sendHeader("Cache-Control", "no-store");
  servidor.send(204);
}

void pagina() {
  servidor.sendHeader("Cache-Control", "no-store");
  servidor.send_P(200, "text/html; charset=utf-8", PAINEL_HTML);
}

void estado() {
  static char json[1536];
  const size_t n = carrinho.estadoJson(json, sizeof(json));
  servidor.sendHeader("Cache-Control", "no-store");
  if (n == 0) {
    servidor.send(500, "application/json", "{\"erro\":\"estado grande demais\"}");
    return;
  }
  servidor.send(200, "application/json", json);
}

void mover() {
  const String d = servidor.arg("d");
  Direcao dir;
  if (d == "frente") dir = Direcao::FRENTE;
  else if (d == "re") dir = Direcao::RE;
  else if (d == "esquerda") dir = Direcao::ESQUERDA;
  else if (d == "direita") dir = Direcao::DIREITA;
  else if (d == "parar") dir = Direcao::PARADO;
  else return servidor.send(400, "text/plain", "d deve ser frente, re, esquerda, direita ou parar");
  carrinho.comandoManual(dir);
  responderOk();
}

void modo() {
  const String m = servidor.arg("m");
  if (m == "manual") carrinho.definirModo(Modo::MANUAL);
  else if (m == "autonomo") carrinho.definirModo(Modo::AUTONOMO);
  else return servidor.send(400, "text/plain", "m deve ser manual ou autonomo");
  responderOk();
}

void letreiro() {
  const String m = servidor.arg("m");
  if (m == "automatico") carrinho.definirLetreiro(ModoLetreiro::AUTOMATICO);
  else if (m == "ligado") carrinho.definirLetreiro(ModoLetreiro::LIGADO);
  else if (m == "desligado") carrinho.definirLetreiro(ModoLetreiro::DESLIGADO);
  else return servidor.send(400, "text/plain", "m deve ser automatico, ligado ou desligado");
  responderOk();
}

void buzina() {
  carrinho.buzina();
  responderOk();
}

void consultar() {
  carrinho.consultarPonto();
  responderOk();
}

void gravar() {
  carrinho.gravar(servidor.arg("ligar") == "1");
  responderOk();
}

void coletaCsv() {
  servidor.sendHeader("Content-Disposition", "attachment; filename=\"coleta.csv\"");
  servidor.sendHeader("Cache-Control", "no-store");
  servidor.setContentLength(CONTENT_LENGTH_UNKNOWN);
  servidor.send(200, "text/csv", "");
  servidor.sendContent("timestamp,dist_cm\n");
  char bloco[1024];
  size_t usado = 0;
  const int n = carrinho.amostrasGravadas();
  for (int i = 0; i < n; i++) {
    usado += snprintf(bloco + usado, sizeof(bloco) - usado, "%d,%.1f\n",
                      i * static_cast<int>(config::PERIODO_LEITURA_MS), carrinho.amostra(i));
    if (usado > sizeof(bloco) - 32) {
      servidor.sendContent(bloco, usado);
      usado = 0;
    }
  }
  if (usado) servidor.sendContent(bloco, usado);
  servidor.sendContent("");  // fim do envio em partes
}
}  // namespace

void Painel::iniciar() {
  servidor.on("/", HTTP_GET, pagina);
  servidor.on("/estado", HTTP_GET, estado);
  servidor.on("/mover", HTTP_POST, mover);
  servidor.on("/modo", HTTP_POST, modo);
  servidor.on("/letreiro", HTTP_POST, letreiro);
  servidor.on("/buzina", HTTP_POST, buzina);
  servidor.on("/consultar", HTTP_POST, consultar);
  servidor.on("/gravar", HTTP_POST, gravar);
  servidor.on("/coleta.csv", HTTP_GET, coletaCsv);
  servidor.onNotFound([] { servidor.send(404, "text/plain", "não encontrado"); });
  servidor.begin();
}

void Painel::atualizar() { servidor.handleClient(); }
