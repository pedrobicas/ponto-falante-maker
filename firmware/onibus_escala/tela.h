// Display OLED SSD1306 128x64: estado do carrinho em texto (sem rostos).
#pragma once

#include <stdint.h>

class Tela {
 public:
  bool iniciar(uint8_t endereco);
  bool ativa() const { return ativa_; }

  // Título grande + até 5 linhas pequenas (21 caracteres cada). Só redesenha se algo mudou.
  void desenhar(const char* titulo, const char* const linhas[], int n);
  // Texto longo (ex.: frase do MVP), quebrado em linhas. Acentos são removidos.
  void desenharTexto(const char* titulo, const char* texto);

 private:
  bool ativa_ = false;
  uint32_t assinatura_ = 0;
};
