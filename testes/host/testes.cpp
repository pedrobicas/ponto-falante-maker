// Testes no PC do firmware do ônibus-escala: unidades portáveis, paridade com o Python e simulação do carrinho
// inteiro (setup/loop reais com stubs do Arduino). Rode com testes/rodar.sh.
#include <cmath>
#include <cstdio>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "../../firmware/onibus_escala/classificador.h"
#include "../../firmware/onibus_escala/config.h"
#include "../../firmware/onibus_escala/controle_luz.h"
#include "../../firmware/onibus_escala/filtro_distancia.h"
#include "../../firmware/onibus_escala/piloto.h"
#include "../../firmware/onibus_escala/texto.h"
#include "sim.h"

void setup();
void loop();

namespace {

int falhas = 0, verificacoes = 0;
std::string grupo;

void checar(bool ok, const std::string& msg) {
  verificacoes++;
  if (!ok) {
    falhas++;
    std::cout << "  FALHOU [" << grupo << "] " << msg << "\n";
  }
}

void inicio(const std::string& nome) {
  grupo = nome;
  std::cout << "- " << nome << "\n";
}

std::string raiz() {
  const char* r = std::getenv("RAIZ_REPO");
  return r ? r : ".";
}

// ---------------------------------------------------------------- filtro
void testeFiltro() {
  inicio("filtro de distância");
  FiltroDistancia f;
  checar(f.aplicar(-1) == 200.0f, "sem eco vira 200 cm");
  checar(f.aplicar(250) == 200.0f, "acima de 200 vira 200 cm");
  f.reiniciar();
  f.aplicar(50);
  f.aplicar(50);
  checar(f.aplicar(1.0f) == 50.0f, "leitura < 2 cm repete a última válida");
  f.reiniciar();
  f.aplicar(40);
  f.aplicar(40);
  checar(f.aplicar(5) == 40.0f, "um salto isolado some na mediana");
  checar(f.aplicar(5) == 5.0f, "dois seguidos passam");
}

// ---------------------------------------------------------------- paridade
std::vector<float> lerSerie(const std::string& arquivo) {
  std::ifstream in(raiz() + "/dataset/csv/" + arquivo);
  std::string linha;
  std::getline(in, linha);
  std::vector<float> v;
  while (std::getline(in, linha)) v.push_back(std::stof(linha.substr(linha.find(',') + 1)));
  return v;
}

void testeParidade() {
  inicio("paridade com ia/treinar.py");
  std::ifstream in(raiz() + "/ia/resultados/paridade.csv");
  checar(in.good(), "ia/resultados/paridade.csv existe (rode ia/treinar.py)");
  std::string linha;
  std::getline(in, linha);
  std::map<std::string, std::vector<float>> series;
  int janelas = 0, iguais = 0;
  double maiorDifAtrib = 0, maiorDifProb = 0;
  while (std::getline(in, linha)) {
    std::stringstream ss(linha);
    std::string campo;
    std::vector<std::string> c;
    while (std::getline(ss, campo, ',')) c.push_back(campo);
    const std::string arquivo = c[0];
    const int inicioJanela = std::stoi(c[1]);
    if (!series.count(arquivo)) series[arquivo] = lerSerie(arquivo);
    const auto& s = series[arquivo];

    Classificador ia;
    for (int i = 0; i < modelo::N_JANELA; i++) ia.adicionar(s[inicioJanela + i]);
    float atrib[modelo::N_ATRIBUTOS];
    ia.atributos(atrib);
    for (int j = 0; j < modelo::N_ATRIBUTOS; j++) {
      const double esperado = std::stod(c[2 + j]);
      maiorDifAtrib = std::max(maiorDifAtrib, std::fabs(atrib[j] - esperado) / std::max(1.0, std::fabs(esperado)));
    }
    const Resultado r = ia.classificar();
    int classePy = 0;
    double maiorPy = -1;
    for (int k = 0; k < modelo::N_CLASSES; k++) {
      const double p = std::stod(c[2 + modelo::N_ATRIBUTOS + k]);
      maiorDifProb = std::max(maiorDifProb, std::fabs(r.prob[k] - p));
      if (p > maiorPy) { maiorPy = p; classePy = k; }
    }
    janelas++;
    if (r.classe == classePy) iguais++;
  }
  std::cout << "  " << janelas << " janelas, classe igual em " << iguais << ", maior diferença: atributos "
            << maiorDifAtrib << ", probabilidade " << maiorDifProb << "\n";
  checar(janelas > 300, "todas as janelas lidas");
  checar(iguais == janelas, "mesma classe que o Python em todas as janelas");
  checar(maiorDifAtrib < 1e-4, "atributos iguais aos do Python");
  checar(maiorDifProb < 1e-4, "probabilidades iguais às do Python");

  Classificador ia;
  checar(!ia.pronto(), "janela vazia não está pronta");
  for (int i = 0; i < modelo::N_JANELA - 1; i++) ia.adicionar(50);
  checar(!ia.pronto(), "14 leituras ainda não bastam");
  ia.adicionar(50);
  checar(ia.pronto(), "15 leituras enchem a janela");
}

// ---------------------------------------------------------------- luz
void testeLuz() {
  inicio("letreiro (LDR com histerese)");
  ControleLuz luz(1200, 1600, 0.2f, false);
  bool aceso = luz.atualizar(3000);
  checar(!aceso, "claro: apagado");
  for (int i = 0; i < 30; i++) aceso = luz.atualizar(800);
  checar(aceso, "escuro: aceso");
  for (int i = 0; i < 30; i++) aceso = luz.atualizar(1400);
  checar(aceso, "luz intermediária (1400) não apaga: histerese");
  for (int i = 0; i < 30; i++) aceso = luz.atualizar(2000);
  checar(!aceso, "claro de novo: apaga");
  luz.definirModo(ModoLetreiro::LIGADO);
  checar(luz.aceso(), "modo ligado força aceso");
  luz.definirModo(ModoLetreiro::DESLIGADO);
  for (int i = 0; i < 30; i++) aceso = luz.atualizar(100);
  checar(!aceso, "modo desligado ignora o escuro");
  ControleLuz inv(1200, 1600, 1.0f, true);
  checar(inv.atualizar(4000), "LDR invertido: leitura alta = escuro");
}

// ---------------------------------------------------------------- piloto
void testePiloto() {
  inicio("piloto autônomo");
  const ParametrosPiloto p = {190, 140, 0.6f, 20.0f, 8.0f, 2, 1000};
  Piloto pil(p);
  pil.reiniciar(0);
  uint32_t t = 0;
  Comando c = pil.atualizar(200, SEM_CLASSE, 0, t);
  checar(c.estado == Estado::AGUARDANDO && c.velocidade == 0, "sem janela: aguarda parado");
  c = pil.atualizar(200, LIVRE, 0.9f, t += 200);
  checar(c.estado == Estado::AGUARDANDO, "um 'livre' ainda não basta");
  c = pil.atualizar(200, LIVRE, 0.9f, t += 200);
  checar(c.estado == Estado::SEGUINDO && c.velocidade == 190, "dois 'livre': segue em velocidade de cruzeiro");
  c = pil.atualizar(60, APROXIMANDO, 0.5f, t += 200);
  checar(c.estado == Estado::SEGUINDO, "confiança baixa mantém a decisão");
  c = pil.atualizar(60, APROXIMANDO, 0.9f, t += 200);
  checar(c.estado == Estado::REDUZINDO && c.velocidade == 140 && c.aviso == Aviso::ATENCAO,
         "aproximando: reduz e avisa");
  c = pil.atualizar(50, APROXIMANDO, 0.9f, t += 200);
  checar(c.aviso == Aviso::NENHUM, "o aviso só toca ao entrar no estado");
  c = pil.atualizar(19, SEM_CLASSE, 0, t += 100);
  checar(c.estado == Estado::PARADO && c.velocidade == 0 && c.aviso == Aviso::PARADA, "regra fixa: para a 20 cm");
  c = pil.atualizar(30, OBSTACULO, 0.9f, t += 100);
  checar(c.estado == Estado::PARADO, "obstáculo parado à frente: continua parado");
  c = pil.atualizar(200, LIVRE, 0.9f, t += 200);
  c = pil.atualizar(200, LIVRE, 0.9f, t += 200);
  checar(c.estado == Estado::PARADO, "não retoma antes de 1 s parado");
  c = pil.atualizar(200, LIVRE, 0.9f, t += 700);
  checar(c.estado == Estado::SEGUINDO, "caminho livre depois de 1 s: retoma");
  c = pil.atualizar(7, SEM_CLASSE, 0, t += 100);
  checar(c.estado == Estado::FREIO && c.aviso == Aviso::FREIO && c.velocidade == 0, "regra fixa: freio a 8 cm");
  c = pil.atualizar(40, SEM_CLASSE, 0, t += 100);
  checar(c.estado == Estado::PARADO && c.velocidade == 0, "saiu do freio: fica parado até a IA liberar");
  c = pil.atualizar(40, LIVRE, 0.95f, t += 100);
  c = pil.atualizar(200, LIVRE, 0.95f, t += 1000);
  checar(c.estado == Estado::SEGUINDO, "IA libera: segue");
  c = pil.atualizar(150, OBSTACULO, 0.9f, t += 200);
  checar(c.estado == Estado::PARADO, "IA vê obstáculo: para mesmo longe da regra fixa");
  c = pil.atualizar(40, APROXIMANDO, 0.9f, t += 200);
  checar(c.estado == Estado::PARADO, "parado e alguém se aproxima: continua parado");
  pil.reiniciar(t);
  c = pil.atualizar(60, APROXIMANDO, 0.9f, t += 200);
  checar(c.estado == Estado::AGUARDANDO && c.velocidade == 0, "aguardando não arranca com algo se aproximando");
}

// ---------------------------------------------------------------- texto
void testeTexto() {
  inicio("texto (JSON e acentos)");
  char s[128];
  checar(texto::campoJson("{\"frase_falada\":\"Linha 875A-10 em 4 minutos.\",\"fonte\":null}", "frase_falada", s,
                          sizeof(s)) && std::string(s) == "Linha 875A-10 em 4 minutos.",
         "lê campo simples");
  checar(texto::campoJson("{ \"a\" : \"x\\\"y\\\\z\\/w\" }", "a", s, sizeof(s)) && std::string(s) == "x\"y\\z/w",
         "trata escapes");
  checar(texto::campoJson("{\"a\":\"\\u00f4nibus \\u00e9\"}", "a", s, sizeof(s)) && std::string(s) == "ônibus é",
         "converte \\u para UTF-8");
  checar(!texto::campoJson("{\"frase_falada\":null}", "frase_falada", s, sizeof(s)), "null não é texto");
  checar(!texto::campoJson("{\"outra\":\"x\"}", "frase_falada", s, sizeof(s)), "campo ausente");
  checar(texto::campoJson("{\"xfrase\":\"no\",\"frase\":\"sim\"}", "frase", s, sizeof(s)) && std::string(s) == "sim",
         "não confunde com nome parecido");
  char pequeno[6];
  checar(texto::campoJson("{\"a\":\"abcdefghij\"}", "a", pequeno, sizeof(pequeno)) && std::string(pequeno) == "abcde",
         "corta sem estourar o buffer");
  checar(texto::escaparJson("diz \"oi\"\\", s, sizeof(s)) && std::string(s) == "\"diz \\\"oi\\\"\\\\\"", "escapa JSON");
  checar(texto::escaparJson("abc", s, 4) == 0, "escapar devolve 0 se não couber");
  char corpo[256];
  checar(texto::corpoEvento("esp32-onibus-escala-01", 70016916, "BOTAO_PRESSIONADO", "2026-11-18T15:04:05-03:00",
                            corpo, sizeof(corpo)) > 0,
         "monta o corpo do evento");
  std::ofstream(raiz() + "/testes/host/saida_evento.json") << corpo;
  texto::semAcentos("Próximo ônibus: São João, ação", s, sizeof(s));
  checar(std::string(s) == "Proximo onibus: Sao Joao, acao", "remove acentos");
}

// ---------------------------------------------------------------- simulação
uint32_t pwmA() { return sim::pwm[config::PINO_ENA]; }
bool frente() {
  return sim::nivel[config::PINO_IN1] == 1 && sim::nivel[config::PINO_IN2] == 0 && sim::nivel[config::PINO_IN3] == 1 &&
         sim::nivel[config::PINO_IN4] == 0;
}
std::string estadoJson() { return sim::requisicao("GET", "/estado").corpo; }
bool contem(const std::string& s, const std::string& trecho) { return s.find(trecho) != std::string::npos; }

// Avança o relógio rodando o loop() real a cada milissegundo. `mundo` muda a distância com o tempo.
void rodar(uint32_t ms, const std::function<void(uint32_t)>& mundo = nullptr, const std::function<void()>& aCada = nullptr) {
  for (uint32_t i = 0; i < ms; i++) {
    sim::agoraMs++;
    if (mundo) mundo(i);
    loop();
    if (aCada) aCada();
  }
}

bool tocou(unsigned hz) {
  for (unsigned t : sim::tons)
    if (t == hz) return true;
  return false;
}

void testeSimulacao() {
  inicio("simulação do carrinho inteiro (setup/loop reais)");
  sim::distanciaCm = -1;  // nada à frente
  sim::ldr = 3000;
  setup();
  rodar(500);
  std::string e = estadoJson();
  checar(contem(e, "\"modo\":\"manual\""), "liga em modo manual");
  checar(pwmA() == 0, "liga parado");
  checar(contem(sim::serial, "[BOOT] pronto"), "boot completo");
  {
    std::ofstream(raiz() + "/testes/host/saida_estado.json") << e;
  }

  // Manual com homem-morto
  sim::requisicao("POST", "/mover", {{"d", "frente"}});
  rodar(100);
  checar(frente() && pwmA() == config::VEL_MANUAL, "manual: anda para a frente");
  rodar(config::MANUAL_TIMEOUT_MS + 50);
  checar(pwmA() == 0, "manual: sem comando do painel, para sozinho");
  checar(sim::requisicao("POST", "/mover", {{"d", "voar"}}).codigo == 400, "direção inválida: 400");

  // Autônomo: caminho livre
  sim::tons.clear();
  checar(sim::requisicao("POST", "/modo", {{"m", "autonomo"}}).codigo == 204, "troca para autônomo");
  rodar(2500);
  checar(frente() && pwmA() == config::VEL_CRUZEIRO, "autônomo, caminho livre: segue em cruzeiro");

  // Algo se aproxima de 100 cm até 18 cm a 40 cm/s e fica parado ali
  bool reduziu = false;
  float distQuandoReduziu = -1, menorAndando = 1000;
  rodar(
      4000,
      [](uint32_t i) { sim::distanciaCm = std::max(18.0f, 100.0f - 0.04f * i); },
      [&] {
        if (!reduziu && pwmA() == config::VEL_REDUZIDA) { reduziu = true; distQuandoReduziu = sim::distanciaCm; }
        if (pwmA() > 0) menorAndando = std::min(menorAndando, sim::distanciaCm);
      });
  std::cout << "  aproximação a 40 cm/s: reduziu a " << distQuandoReduziu << " cm, menor distância andando "
            << menorAndando << " cm\n";
  checar(reduziu, "aproximação: reduz a velocidade antes de parar");
  checar(distQuandoReduziu > 30, "reduz com folga (antes de 30 cm)");
  // O filtro (mediana de 3) atrasa ~1 leitura: a 40 cm/s são ~4 cm além da regra de 20 cm.
  checar(menorAndando >= config::DIST_PARADA_CM - 5, "para perto da regra de 20 cm, longe do freio de 8 cm");
  checar(pwmA() == 0, "objeto parado a 18 cm: fica parado");
  checar(tocou(900) && tocou(1200), "tocou o aviso de atenção e o de parada");

  // Objeto sai: volta a andar
  sim::distanciaCm = -1;
  rodar(3000);
  checar(frente() && pwmA() == config::VEL_CRUZEIRO, "caminho liberado: retoma");

  // Algo aparece colado: freio
  sim::tons.clear();
  sim::distanciaCm = 6;
  rodar(400);
  checar(pwmA() == 0 && tocou(1800), "a 6 cm: freio de emergência com alarme");
  sim::distanciaCm = -1;
  rodar(3000);
  checar(pwmA() == config::VEL_CRUZEIRO, "depois do freio, só volta com caminho livre");

  // Loop travado: janela descartada
  sim::agoraMs += 1000;
  rodar(10);
  checar(contem(sim::serial, "janela descartada"), "loop atrasado: descarta a janela");
  rodar(2500);
  checar(pwmA() == config::VEL_CRUZEIRO, "recupera depois de reencher a janela");

  // Letreiro
  sim::ldr = 600;
  rodar(1500);
  checar(sim::nivel[config::PINO_LETREIRO] == 1, "pouca luz: letreiro aceso");
  sim::ldr = 1400;
  rodar(1500);
  checar(sim::nivel[config::PINO_LETREIRO] == 1, "luz intermediária: continua aceso");
  sim::ldr = 3000;
  rodar(1500);
  checar(sim::nivel[config::PINO_LETREIRO] == 0, "luz volta: apaga");
  sim::requisicao("POST", "/letreiro", {{"m", "ligado"}});
  rodar(200);
  checar(sim::nivel[config::PINO_LETREIRO] == 1, "painel força o letreiro ligado");
  sim::requisicao("POST", "/letreiro", {{"m", "automatico"}});
  rodar(200);
  checar(sim::nivel[config::PINO_LETREIRO] == 0, "volta ao automático");

  // Botão BOOT: segurar 1 s alterna o modo
  sim::botaoApertado = true;
  rodar(1100);
  sim::botaoApertado = false;
  rodar(50);
  checar(contem(estadoJson(), "\"modo\":\"manual\""), "segurar o BOOT 1 s: volta ao manual");
  checar(pwmA() == 0, "trocar de modo para os motores");
  // Toque curto: consulta o ponto (sem credenciais, avisa no display)
  sim::botaoApertado = true;
  rodar(100);
  sim::botaoApertado = false;
  rodar(400);
  bool proximos = !sim::tela.empty() && sim::tela[0] == "PROXIMOS";
  checar(proximos, "toque no BOOT: display mostra a resposta da consulta");
  checar(contem(estadoJson(), "credenciais.h"), "sem credenciais: o painel explica por quê");

  // Coleta
  sim::requisicao("POST", "/gravar", {{"ligar", "1"}});
  sim::distanciaCm = 42.0f;
  rodar(2000);
  sim::requisicao("POST", "/gravar", {{"ligar", "0"}});
  const std::string csv = sim::requisicao("GET", "/coleta.csv").corpo;
  int linhas = 0;
  for (char ch : csv) linhas += ch == '\n';
  checar(csv.rfind("timestamp,dist_cm\n0,", 0) == 0, "CSV no formato do dataset");
  checar(linhas >= 20 && linhas <= 22, "2 s gravados = ~20 amostras");
  checar(contem(csv, ",42.0\n"), "valores gravados");

  checar(sim::requisicao("GET", "/").corpo.find("<!doctype html>") == 0, "painel servido em /");
  checar(sim::requisicao("GET", "/nada").codigo == 404, "rota desconhecida: 404");
  {
    std::ofstream(raiz() + "/testes/host/saida_estado.json") << estadoJson();
  }
}

}  // namespace

int main() {
  testeFiltro();
  testeParidade();
  testeLuz();
  testePiloto();
  testeTexto();
  testeSimulacao();
  std::cout << "\n" << verificacoes - falhas << "/" << verificacoes << " verificações passaram\n";
  return falhas ? 1 : 0;
}
