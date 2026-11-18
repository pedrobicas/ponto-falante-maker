#include "tela.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <string.h>

#include "texto.h"

namespace {
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
constexpr int COLUNAS = 21;

uint32_t hashTexto(uint32_t h, const char* s) {
  while (s && *s) h = (h ^ static_cast<uint8_t>(*s++)) * 16777619u;  // FNV-1a
  return (h ^ 0xFF) * 16777619u;
}
}  // namespace

bool Tela::iniciar(uint8_t endereco) {
  Wire.begin();
  Wire.setClock(400000);  // redesenho em ~25 ms em vez de ~100 ms
  ativa_ = oled.begin(SSD1306_SWITCHCAPVCC, endereco);
  if (ativa_) {
    oled.clearDisplay();
    oled.setTextColor(SSD1306_WHITE);
    oled.setTextWrap(false);
    oled.display();
  }
  return ativa_;
}

void Tela::desenhar(const char* titulo, const char* const linhas[], int n) {
  if (!ativa_) return;
  uint32_t h = hashTexto(2166136261u, titulo);
  for (int i = 0; i < n; i++) h = hashTexto(h, linhas[i]);
  if (h == assinatura_) return;
  assinatura_ = h;

  oled.clearDisplay();
  oled.setTextSize(2);
  oled.setCursor(0, 0);
  oled.print(titulo);
  oled.setTextSize(1);
  for (int i = 0; i < n && i < 5; i++) {
    oled.setCursor(0, 20 + i * 9);
    oled.print(linhas[i]);
  }
  oled.display();
}

void Tela::desenharTexto(const char* titulo, const char* texto) {
  if (!ativa_) return;
  char limpo[COLUNAS * 6 + 1];
  texto::semAcentos(texto, limpo, sizeof(limpo));

  // Quebra por palavras em até 5 linhas.
  static char buf[5][COLUNAS + 1];
  const char* linhas[5];
  int n = 0;
  const char* p = limpo;
  while (*p && n < 5) {
    while (*p == ' ') p++;
    int corte = 0, ultimoEspaco = -1;
    while (p[corte] && corte < COLUNAS) {
      if (p[corte] == ' ') ultimoEspaco = corte;
      corte++;
    }
    if (p[corte] && ultimoEspaco > 0) corte = ultimoEspaco;
    memcpy(buf[n], p, corte);
    buf[n][corte] = '\0';
    linhas[n] = buf[n];
    n++;
    p += corte;
  }
  desenhar(titulo, linhas, n);
}
