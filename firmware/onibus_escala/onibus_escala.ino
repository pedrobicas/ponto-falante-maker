// Ponto Falante · ônibus-escala (Project-Based Maker Lab, GS 2026.2)
//
// O carrinho faz o papel do ônibus chegando ao ponto:
//   - IA embarcada (regressão logística sobre a tendência da distância do HC-SR04) decide entre
//     livre / aproximando / obstaculo; o piloto segue, reduz com aviso sonoro ou para;
//   - duas regras fixas ficam acima da IA: para a 20 cm e freia a 8 cm;
//   - o LDR acende o letreiro com pouca luz, para ele continuar legível;
//   - o ESP32 conversa com o MVP pelo contrato de dispositivos (HEARTBEAT e BOTAO_PRESSIONADO);
//   - painel web próprio (Wi-Fi do carrinho, http://192.168.4.1) para pilotar, ver a IA e gravar dados.
//
// Placa: ESP32 Dev Module, pacote esp32 3.x. Biblioteca extra: Adafruit SSD1306 (instala a Adafruit GFX junto).

#include <WiFi.h>

#include "carrinho.h"
#include "cliente_mvp.h"
#include "credenciais_ativas.h"
#include "painel.h"

void iniciarWiFi() {
  const bool comInternet = strlen(WIFI_REDE) > 0;
  WiFi.mode(comInternet ? WIFI_AP_STA : WIFI_AP);
  WiFi.setSleep(false);  // painel e comandos sem atraso
  WiFi.softAP(PAINEL_REDE, PAINEL_SENHA);
  Serial.printf("[WIFI] painel: rede \"%s\", abra http://%s\n", PAINEL_REDE, WiFi.softAPIP().toString().c_str());
  if (comInternet) {
    WiFi.begin(WIFI_REDE, WIFI_SENHA);
    Serial.printf("[WIFI] conectando em \"%s\" para falar com o MVP\n", WIFI_REDE);
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[BOOT] Ponto Falante · ônibus-escala");
  if (!CREDENCIAIS_PROPRIAS) Serial.println("[BOOT] sem credenciais.h: usando credenciais.exemplo.h");

  carrinho.iniciar();
  iniciarWiFi();
  painel.iniciar();
  mvp.iniciar();
  Serial.println("[BOOT] pronto. Botão BOOT: toque = próximos ônibus do ponto; segurar 1 s = manual/autônomo");
}

void loop() {
  painel.atualizar();
  carrinho.atualizar();
}
