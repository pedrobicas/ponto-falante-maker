// Pinos e parâmetros do ônibus-escala. Os pinos seguem a fiação do carrinho (ver README, seção 5).
#pragma once

#include <stdint.h>

namespace config {

// ---------- pinos (ESP32 WROOM-32) ----------
constexpr uint8_t PINO_IN1 = 13;
constexpr uint8_t PINO_IN2 = 4;
constexpr uint8_t PINO_IN3 = 33;
constexpr uint8_t PINO_IN4 = 32;
constexpr uint8_t PINO_ENA = 19;  // PWM lado A
constexpr uint8_t PINO_ENB = 18;  // PWM lado B
constexpr uint8_t PINO_TRIG = 25;
constexpr uint8_t PINO_ECHO = 26;
constexpr uint8_t PINO_LDR = 34;       // ADC1: continua funcionando com o Wi-Fi ligado
constexpr uint8_t PINO_LETREIRO = 27;  // LED que ilumina o letreiro
constexpr uint8_t PINO_BUZZER = 5;
constexpr uint8_t PINO_BOTAO = 0;      // botão BOOT da placa: pede ao MVP os próximos ônibus do ponto
constexpr uint8_t OLED_ENDERECO = 0x3C;  // SSD1306 128x64 no I2C padrão (SDA 21, SCL 22)

// ---------- motores ----------
constexpr uint32_t PWM_FREQ_HZ = 20000;
constexpr uint8_t PWM_BITS = 8;
constexpr int VEL_CRUZEIRO = 190;   // 0..255. Calibre no chão: abaixo de ~120 os motores podem não girar
constexpr int VEL_REDUZIDA = 140;
constexpr int VEL_MANUAL = 200;
constexpr uint32_t MANUAL_TIMEOUT_MS = 600;  // sem comando do painel por esse tempo, para (homem-morto)

// ---------- ultrassom ----------
constexpr uint32_t PERIODO_LEITURA_MS = 100;  // 10 Hz, igual ao dataset
constexpr uint32_t ECO_TIMEOUT_US = 12000;    // ~2 m ida e volta; acima disso vale "sem eco"

// ---------- piloto autônomo ----------
constexpr int INFERIR_A_CADA = 2;           // uma inferência a cada 2 leituras (200 ms)
constexpr float CONFIANCA_MIN = 0.60f;      // abaixo disso, mantém a decisão anterior
constexpr float DIST_PARADA_CM = 20.0f;     // regra fixa: para a esta distância, com ou sem a IA
constexpr float DIST_FREIO_CM = 8.0f;       // regra fixa: freio de emergência
constexpr int LIVRES_PARA_SEGUIR = 2;       // inferências "livre" seguidas para voltar a andar
constexpr uint32_t PARADA_MIN_MS = 1000;    // tempo mínimo parado antes de retomar

// ---------- letreiro (LDR) ----------
constexpr uint32_t PERIODO_LUZ_MS = 100;
constexpr int LUZ_LIGA_ABAIXO = 1200;       // ADC 0..4095: 0 = escuro
constexpr int LUZ_DESLIGA_ACIMA = 1600;     // histerese: não pisca na luz intermediária
constexpr float LUZ_SUAVIZACAO = 0.2f;      // peso da leitura nova na média móvel
constexpr bool LDR_INVERTIDO = false;       // true se o LDR estiver do lado do GND

// ---------- integração com o MVP ----------
constexpr uint32_t HEARTBEAT_MS = 60000;
constexpr uint32_t HTTP_TIMEOUT_MS = 20000;  // o Render gratuito pode levar ~1 min para acordar
constexpr long FUSO_S = -3 * 3600;           // Brasília

// ---------- coleta de dados ----------
constexpr int COLETA_MAX_AMOSTRAS = 1200;   // 2 min a 10 Hz

}  // namespace config
