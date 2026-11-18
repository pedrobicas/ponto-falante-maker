// Tratamento da leitura do HC-SR04. É o mesmo tratamento aplicado ao dataset, para a IA ver no carrinho
// dados iguais aos do treino. Sem dependência do Arduino (testado no PC em testes/host).
#pragma once

class FiltroDistancia {
 public:
  static constexpr float SEM_ECO_CM = 200.0f;
  static constexpr float MINIMO_CM = 2.0f;  // abaixo disso o HC-SR04 não mede: é ruído

  // bruta < 0 significa "sem eco". Devolve a mediana das 3 últimas leituras tratadas.
  float aplicar(float bruta);
  void reiniciar();

 private:
  float ultima_ = SEM_ECO_CM;
  float historico_[3] = {SEM_ECO_CM, SEM_ECO_CM, SEM_ECO_CM};
};
