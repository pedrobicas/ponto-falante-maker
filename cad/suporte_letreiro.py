"""Gera a peça modelada do ônibus-escala: caixa de luz do letreiro, com LED atrás e LDR ao lado.

Saída: cad/suporte_letreiro.stl
Uso: pip install trimesh manifold3d && python cad/suporte_letreiro.py
Unidades em mm. Eixos: X largura, Y profundidade (frente = -Y), Z altura.

Como funciona: o letreiro (tira de papel ou acetato impressa, 76 x 22 mm) entra pela fenda de cima e fica
preso entre a moldura da frente e um ressalto. O LED de 5 mm entra pelo furo de trás e ilumina a câmara.
O LDR fica na torre lateral, fora da câmara, para medir a luz do ambiente e não a do próprio LED.
"""

from __future__ import annotations

from pathlib import Path

import numpy as np
import trimesh

RAIZ = Path(__file__).resolve().parents[1]

# ---------------- parâmetros ----------------
LARGURA, PROFUNDIDADE, ALTURA = 84.0, 18.0, 26.0
PAREDE = 2.0
JANELA = (70.0, 16.0)          # abertura visível do letreiro
FENDA = (76.0, 1.2)            # largura x espessura do encaixe do letreiro
RESSALTO_ABERTURA = (72.0, 18.0)
FURO_LED = 5.2
FURO_LDR = 5.2
FURO_FIOS = 2.5
FURO_M3 = 3.4
ABA = 14.0                     # comprimento das abas de fixação
ESPESSURA_ABA = 3.0


def caixa(x0, x1, y0, y1, z0, z1):
    return trimesh.creation.box(bounds=[[x0, y0, z0], [x1, y1, z1]])


def cilindro_z(raio, z0, z1, x, y):
    c = trimesh.creation.cylinder(radius=raio, height=z1 - z0, sections=48)
    c.apply_translation([x, y, (z0 + z1) / 2])
    return c


def cilindro_y(raio, y0, y1, x, z):
    c = trimesh.creation.cylinder(radius=raio, height=y1 - y0, sections=48)
    c.apply_transform(trimesh.transformations.rotation_matrix(np.pi / 2, [1, 0, 0]))
    c.apply_translation([x, (y0 + y1) / 2, z])
    return c


def gerar() -> trimesh.Trimesh:
    w2, d2 = LARGURA / 2, PROFUNDIDADE / 2
    y_moldura = -d2 + 1.5                 # moldura da frente: 1,5 mm
    y_fenda = y_moldura + FENDA[1]        # fenda do letreiro
    y_ressalto = y_fenda + 1.0            # ressalto: 1 mm
    y_fundo = d2 - PAREDE                 # parede de trás
    zc = ALTURA / 2

    corpo = caixa(-w2, w2, -d2, d2, 0, ALTURA)
    # abas de fixação nos dois lados e torre do LDR à direita
    abas = [caixa(-w2 - ABA, -w2, -d2, d2, 0, ESPESSURA_ABA), caixa(w2, w2 + ABA, -d2, d2, 0, ESPESSURA_ABA)]
    torre = caixa(w2, w2 + 8, -4, 4, 0, ALTURA)
    peca = trimesh.boolean.union([corpo, *abas, torre])

    vazios = [
        # câmara de luz
        caixa(-w2 + PAREDE, w2 - PAREDE, y_ressalto, y_fundo, PAREDE, ALTURA - PAREDE),
        # abertura do ressalto
        caixa(-RESSALTO_ABERTURA[0] / 2, RESSALTO_ABERTURA[0] / 2, y_fenda - 0.01, y_ressalto + 0.01,
              zc - RESSALTO_ABERTURA[1] / 2, zc + RESSALTO_ABERTURA[1] / 2),
        # fenda do letreiro, aberta em cima
        caixa(-FENDA[0] / 2, FENDA[0] / 2, y_moldura, y_fenda, PAREDE, ALTURA + 1),
        # janela da frente
        caixa(-JANELA[0] / 2, JANELA[0] / 2, -d2 - 1, y_moldura + 0.01, zc - JANELA[1] / 2, zc + JANELA[1] / 2),
        # furo do LED na parede de trás (eixo Y)
        cilindro_y(FURO_LED / 2, y_fundo - 0.5, d2 + 1, 0, zc),
        # LDR: cavidade de cima na torre + passagem dos fios até embaixo
        cilindro_z(FURO_LDR / 2, ALTURA - 8, ALTURA + 1, w2 + 4, 0),
        cilindro_z(FURO_FIOS / 2, -1, ALTURA - 7.9, w2 + 4, 0),
        # furos M3 das abas
        cilindro_z(FURO_M3 / 2, -1, ESPESSURA_ABA + 1, -w2 - ABA / 2, 0),
        cilindro_z(FURO_M3 / 2, -1, ESPESSURA_ABA + 1, w2 + ABA - 3.5, 0),
    ]
    return trimesh.boolean.difference([peca, *vazios])


def main():
    m = gerar()
    assert m.is_watertight, "a malha precisa ser fechada para fatiar"
    (RAIZ / "cad").mkdir(exist_ok=True)
    m.export(RAIZ / "cad" / "suporte_letreiro.stl")
    print(f"STL: {len(m.faces)} triângulos, {m.extents.round(1)} mm, volume {m.volume / 1000:.1f} cm³, fechada: {m.is_watertight}")


if __name__ == "__main__":
    main()
