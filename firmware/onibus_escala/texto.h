// Utilidades de texto sem dependência do Arduino (testadas no PC em testes/host).
#pragma once

#include <stddef.h>

namespace texto {

// Copia para `saida` o valor do campo de texto `campo` de um JSON simples (sem objetos aninhados com o mesmo
// nome). Trata \" \\ \/ \n \t e \uXXXX (vira UTF-8). Devolve false se o campo não existir ou não for texto.
bool campoJson(const char* json, const char* campo, char* saida, size_t tamanho);

// Escreve `entrada` entre aspas como texto JSON (escapa aspas, barras e controles).
// Devolve o número de bytes escritos (sem o \0), ou 0 se não couber.
size_t escaparJson(const char* entrada, char* saida, size_t tamanho);

// Corpo JSON do POST /api/integracoes/dispositivos/eventos (schema EventoDispositivo do MVP).
// `quando` em ISO 8601 com fuso, ex.: 2026-11-18T15:04:05-03:00. Devolve bytes escritos, ou 0 se não couber.
size_t corpoEvento(const char* dispositivoId, long ponto, const char* tipo, const char* quando, char* saida,
                   size_t tamanho);

// Troca letras acentuadas em UTF-8 pela letra sem acento (a fonte do display não tem acentos).
void semAcentos(const char* entrada, char* saida, size_t tamanho);

}  // namespace texto
