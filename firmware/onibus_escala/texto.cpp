#include "texto.h"

#include <stdio.h>
#include <string.h>

namespace texto {

namespace {

int hex(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

// Escreve o código Unicode em UTF-8. Devolve bytes escritos (0 se não couber).
size_t utf8(unsigned cp, char* s, size_t livre) {
  if (cp < 0x80 && livre >= 1) { s[0] = static_cast<char>(cp); return 1; }
  if (cp < 0x800 && livre >= 2) {
    s[0] = static_cast<char>(0xC0 | (cp >> 6));
    s[1] = static_cast<char>(0x80 | (cp & 0x3F));
    return 2;
  }
  if (cp >= 0x800 && livre >= 3) {
    s[0] = static_cast<char>(0xE0 | (cp >> 12));
    s[1] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    s[2] = static_cast<char>(0x80 | (cp & 0x3F));
    return 3;
  }
  return 0;
}

const char* pularEspacos(const char* p) {
  while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
  return p;
}

}  // namespace

bool campoJson(const char* json, const char* campo, char* saida, size_t tamanho) {
  if (!json || !campo || !saida || tamanho == 0) return false;
  saida[0] = '\0';
  const size_t n = strlen(campo);
  for (const char* p = strchr(json, '"'); p; p = strchr(p + 1, '"')) {
    if (strncmp(p + 1, campo, n) != 0 || p[n + 1] != '"') continue;
    const char* v = pularEspacos(p + n + 2);
    if (*v != ':') continue;
    v = pularEspacos(v + 1);
    if (*v != '"') return false;  // null, número etc.
    v++;
    size_t o = 0;
    while (*v && *v != '"') {
      char c = *v++;
      if (c == '\\') {
        const char e = *v++;
        if (e == 'u') {
          unsigned cp = 0;
          for (int i = 0; i < 4; i++) {
            const int h = hex(v[i]);
            if (h < 0) return false;
            cp = cp * 16 + h;
          }
          v += 4;
          const size_t w = utf8(cp, saida + o, tamanho - 1 - o);
          if (w == 0) break;
          o += w;
          continue;
        }
        c = e == 'n' ? ' ' : e == 't' ? ' ' : e == 'r' ? ' ' : e;  // \" \\ \/ ficam como estão
        if (!e) break;
      }
      if (o + 1 >= tamanho) break;
      saida[o++] = c;
    }
    saida[o] = '\0';
    return true;
  }
  return false;
}

size_t escaparJson(const char* entrada, char* saida, size_t tamanho) {
  size_t o = 0;
  auto por = [&](char c) {
    if (o + 1 >= tamanho) return false;
    saida[o++] = c;
    return true;
  };
  if (!por('"')) return 0;
  for (const char* p = entrada; *p; p++) {
    const unsigned char c = static_cast<unsigned char>(*p);
    bool ok = true;
    if (c == '"' || c == '\\') ok = por('\\') && por(static_cast<char>(c));
    else if (c < 0x20) ok = por(' ');
    else ok = por(static_cast<char>(c));
    if (!ok) return 0;
  }
  if (!por('"')) return 0;
  saida[o] = '\0';
  return o;
}

size_t corpoEvento(const char* dispositivoId, long ponto, const char* tipo, const char* quando, char* saida,
                   size_t tamanho) {
  char id[80];
  if (!escaparJson(dispositivoId, id, sizeof(id))) return 0;
  const int n = snprintf(saida, tamanho,
                         "{\"dispositivo_id\":%s,\"ponto\":%ld,\"tipo\":\"%s\",\"ocorrido_em\":\"%s\",\"bateria_pct\":null}",
                         id, ponto, tipo, quando);
  return n > 0 && static_cast<size_t>(n) < tamanho ? static_cast<size_t>(n) : 0;
}

void semAcentos(const char* entrada, char* saida, size_t tamanho) {
  // Pares UTF-8 (C3 xx) -> ASCII. Cobre o português.
  static const char* const de = "\xA0\xA1\xA2\xA3\xA4\xA7\xA8\xA9\xAA\xAD\xB3\xB4\xB5\xBA\xBC"
                                "\x80\x81\x82\x83\x87\x89\x8A\x8D\x93\x94\x95\x9A\x9C";
  static const char* const para = "aaaaaceeeiooouu" "AAAACEEIOOOUU";
  // de/para têm o mesmo tamanho (28), alinhados por posição.
  static_assert(sizeof("aaaaaceeeiooouu" "AAAACEEIOOOUU") == 29, "tabela de acentos");
  size_t o = 0;
  for (const unsigned char* p = reinterpret_cast<const unsigned char*>(entrada); *p && o + 1 < tamanho; p++) {
    if (*p == 0xC3 && p[1]) {
      const char* achou = strchr(de, static_cast<char>(p[1]));
      saida[o++] = achou ? para[achou - de] : '?';
      p++;
    } else if (*p == 0xC2 && p[1]) {
      p++;  // º, ª, espaço rígido etc.
      saida[o++] = p[0] == 0xBA ? 'o' : p[0] == 0xAA ? 'a' : ' ';
    } else if (*p >= 0x80) {
      // outros caracteres multibyte: pula os bytes de continuação
      while ((p[1] & 0xC0) == 0x80) p++;
      saida[o++] = '?';
    } else {
      saida[o++] = static_cast<char>(*p);
    }
  }
  saida[o] = '\0';
}

}  // namespace texto
