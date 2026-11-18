"""Valida as saídas JSON do teste do firmware.

1. /estado: JSON válido com os campos que o painel usa.
2. Corpo do evento: segue o schema EventoDispositivo de contratos/openapi-mvp.yaml do Ponto Falante
   (copiado abaixo; campos em snake_case porque o Spring usa SNAKE_CASE).
"""

import json
import re
import sys
from datetime import datetime

estado = json.load(open(sys.argv[1], encoding="utf-8"))
for campo in ["modo", "estado", "distancia", "classe", "prob", "luz", "letreiro", "mvp", "gravando"]:
    assert campo in estado, f"/estado sem {campo}"
assert len(estado["prob"]) == 3
print("JSON do /estado válido")

# EventoDispositivo (openapi-mvp.yaml): required [dispositivo_id, ponto, tipo, ocorrido_em]
evento = json.load(open(sys.argv[2], encoding="utf-8"))
assert set(evento) <= {"dispositivo_id", "ponto", "tipo", "ocorrido_em", "bateria_pct"}, evento
assert isinstance(evento["dispositivo_id"], str) and 0 < len(evento["dispositivo_id"]) <= 64
assert isinstance(evento["ponto"], int)
assert evento["tipo"] in {"BOTAO_PRESSIONADO", "HEARTBEAT"}
assert re.fullmatch(r"\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}[+-]\d{2}:\d{2}", evento["ocorrido_em"])
datetime.fromisoformat(evento["ocorrido_em"])
assert evento.get("bateria_pct") is None or 0 <= evento["bateria_pct"] <= 100
print("corpo do evento segue o contrato EventoDispositivo do MVP")
