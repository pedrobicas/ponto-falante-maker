#!/usr/bin/env bash
# Compila o firmware no PC com stubs do Arduino-ESP32 e roda os testes.
# Uso: bash testes/rodar.sh   (precisa de g++ e python3)
set -euo pipefail

RAIZ="$(cd "$(dirname "$0")/.." && pwd)"
FW="$RAIZ/firmware/onibus_escala"
OUT="$(mktemp -d)"
trap 'rm -rf "$OUT"' EXIT

# O Arduino IDE compila o .ino como C++ com #include <Arduino.h> no topo; aqui fazemos o mesmo.
{ echo '#include <Arduino.h>'; cat "$FW/onibus_escala.ino"; } > "$OUT/onibus_escala_ino.cpp"

g++ -std=c++17 -O1 -Wall -Wextra -Werror -Wno-unused-parameter \
  -I "$RAIZ/testes/stubs" -I "$FW" \
  "$FW"/*.cpp "$OUT/onibus_escala_ino.cpp" "$RAIZ/testes/stubs/stubs.cpp" "$RAIZ/testes/host/testes.cpp" \
  -o "$OUT/testes"

RAIZ_REPO="$RAIZ" "$OUT/testes"

# O JSON do /estado precisa ser válido para o painel, e o corpo do evento precisa seguir o contrato do MVP.
python3 "$RAIZ/testes/validar_json.py" "$RAIZ/testes/host/saida_estado.json" "$RAIZ/testes/host/saida_evento.json"
rm -f "$RAIZ/testes/host/saida_estado.json" "$RAIZ/testes/host/saida_evento.json"
