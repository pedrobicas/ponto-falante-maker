"""Serve o painel do carrinho no PC, com um carrinho de mentira, para testar a página sem o ESP32.

Uso: python testes/painel/servidor_simulado.py  e abra http://127.0.0.1:8080
A página é a mesma do firmware (extraída de firmware/onibus_escala/painel_html.h).
"""

from __future__ import annotations

import json
import math
import re
import sys
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, urlparse

RAIZ = Path(__file__).resolve().parents[2]
HTML = re.search(r'R"HTML\((.*)\)HTML"', (RAIZ / "firmware/onibus_escala/painel_html.h").read_text(encoding="utf-8"),
                 re.S).group(1)

estado = {"modo": "manual", "letreiro_modo": "automatico", "gravando": False, "amostras": 0, "motor": "parado",
          "versao": 0, "frase": "", "chamadas": []}
inicio = time.time()


def snapshot() -> dict:
    t = time.time() - inicio
    dist = 60 + 40 * math.sin(t / 3)
    prob = [0.9, 0.08, 0.02] if dist > 50 else [0.1, 0.8, 0.1]
    classes = ["livre", "aproximando", "obstaculo"]
    auto = estado["modo"] == "autonomo"
    return {
        "modo": estado["modo"], "estado": ("SEGUINDO" if dist > 50 else "REDUZINDO") if auto else "MANUAL",
        "distancia": round(dist, 1), "janela_pronta": True, "classe": classes[prob.index(max(prob))],
        "confianca": max(prob), "prob": prob, "motor": estado["motor"], "velocidade": 190 if auto else 0,
        "luz": 900, "letreiro": estado["letreiro_modo"] != "desligado", "letreiro_modo": estado["letreiro_modo"],
        "gravando": estado["gravando"], "amostras": estado["amostras"], "amostras_max": 1200,
        "mvp": {"configurado": True, "wifi": True, "hora": True, "enviando": False, "codigo": 202,
                "tipo": "HEARTBEAT", "ha_s": 12, "enviados": 3, "frase": estado["frase"], "versao": estado["versao"],
                "ip": "192.168.43.20", "ponto": 70016916, "url": "https://ponto-falante.onrender.com"},
        "uptime_s": int(t),
    }


class Handler(BaseHTTPRequestHandler):
    def log_message(self, *a):  # silencioso
        pass

    def _send(self, code, tipo="text/plain", corpo=b""):
        self.send_response(code)
        self.send_header("Content-Type", tipo)
        self.send_header("Content-Length", str(len(corpo)))
        self.end_headers()
        self.wfile.write(corpo)

    def do_GET(self):
        u = urlparse(self.path)
        if u.path == "/":
            return self._send(200, "text/html; charset=utf-8", HTML.encode())
        if u.path == "/estado":
            return self._send(200, "application/json", json.dumps(snapshot()).encode())
        if u.path == "/coleta.csv":
            linhas = ["timestamp,dist_cm"] + [f"{i * 100},{50.0:.1f}" for i in range(estado["amostras"])]
            return self._send(200, "text/csv", ("\n".join(linhas) + "\n").encode())
        if u.path == "/chamadas":
            return self._send(200, "application/json", json.dumps(estado["chamadas"]).encode())
        return self._send(404)

    def do_POST(self):
        u = urlparse(self.path)
        q = {k: v[0] for k, v in parse_qs(u.query).items()}
        estado["chamadas"].append({"rota": u.path, **q})
        if u.path == "/modo":
            estado["modo"] = q.get("m", "manual")
        elif u.path == "/mover":
            estado["motor"] = q.get("d", "parar").replace("parar", "parado")
            estado["modo"] = "manual"
        elif u.path == "/letreiro":
            estado["letreiro_modo"] = q.get("m", "automatico")
        elif u.path == "/gravar":
            estado["gravando"] = q.get("ligar") == "1"
            estado["amostras"] = 37 if not estado["gravando"] else 0
        elif u.path == "/consultar":
            estado["frase"] = "Ponto Avenida Paulista, 1374. Linha 875A-10, sentido Aeroporto, em 4 minutos."
            estado["versao"] += 1
        return self._send(204)


if __name__ == "__main__":
    porta = int(sys.argv[1]) if len(sys.argv) > 1 else 8080
    print(f"painel simulado em http://127.0.0.1:{porta}")
    ThreadingHTTPServer(("127.0.0.1", porta), Handler).serve_forever()
