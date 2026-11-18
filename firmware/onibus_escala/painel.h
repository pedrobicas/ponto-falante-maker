// Painel web servido pelo ESP32 na rede própria do carrinho (modo AP).
#pragma once

class Painel {
 public:
  void iniciar();
  void atualizar();
};

extern Painel painel;
