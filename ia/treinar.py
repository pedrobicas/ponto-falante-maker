"""Treina o classificador de aproximação do ônibus-escala e exporta o modelo para o firmware.

Entrada: dataset/csv/<classe>.<n>.csv com colunas timestamp,dist_cm (10 Hz, já filtrado como no firmware).
Saídas:
  firmware/onibus_escala/modelo_aproximacao.h   pesos para o ESP32 (gerado, não editar à mão)
  ia/resultados/metricas.json                   validação deixando um arquivo de fora
  ia/resultados/comparacao.md                   configurações testadas
  ia/resultados/matriz_confusao.png
  ia/resultados/paridade.csv                    atributos e probabilidades por janela (teste do C++)

Uso: python ia/treinar.py
"""

from __future__ import annotations

import csv
import json
from dataclasses import dataclass
from pathlib import Path

import numpy as np
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import accuracy_score, confusion_matrix, f1_score, precision_recall_fscore_support
from sklearn.preprocessing import StandardScaler

RAIZ = Path(__file__).resolve().parents[1]
DATASET = RAIZ / "dataset" / "csv"
RESULTADOS = RAIZ / "ia" / "resultados"
CABECALHO = RAIZ / "firmware" / "onibus_escala" / "modelo_aproximacao.h"

CLASSES = ["livre", "aproximando", "obstaculo"]
PERIODO_S = 0.1          # 10 Hz
SEM_ECO_CM = 200.0       # valor que o filtro grava quando não há eco
QUEDA_MIN_APROX_CM = 10  # janela de "aproximando" precisa cair pelo menos isso

# Configuração escolhida (ver comparacao.md)
JANELA = 15              # 1,5 s
LIMITE_CM = 100.0        # acima disso a distância não muda a decisão do carrinho
ATRIBUTOS = ["ultimo", "media", "minimo", "desvio", "inclinacao", "queda", "sem_eco"]


@dataclass
class Serie:
    arquivo: str
    classe: str
    dist: np.ndarray


def carregar() -> list[Serie]:
    series = []
    for caminho in sorted(DATASET.glob("*.csv")):
        classe = caminho.name.split(".")[0]
        if classe not in CLASSES:
            raise ValueError(f"classe desconhecida no nome do arquivo: {caminho.name}")
        dist = np.loadtxt(caminho, delimiter=",", skiprows=1, usecols=1, dtype=np.float64)
        series.append(Serie(caminho.name, classe, dist))
    return series


def atributos(janela: np.ndarray, limite: float = LIMITE_CM) -> list[float]:
    """Mesmo cálculo de classificador.cpp. Mudou aqui, mude lá (o teste de paridade acusa)."""
    x = np.minimum(janela, limite)
    n = len(x)
    t = np.arange(n) * PERIODO_S
    tm, xm = t.mean(), x.mean()
    inclinacao = float(((t - tm) * (x - xm)).sum() / ((t - tm) ** 2).sum())  # cm/s
    queda = float(x[:3].mean() - x[-3:].mean())
    sem_eco = float(np.mean(janela >= SEM_ECO_CM - 0.05))
    return [float(x[-1]), float(xm), float(x.min()), float(x.std()), inclinacao, queda, sem_eco]


def janela_valida(classe: str, janela: np.ndarray, limite: float) -> bool:
    """Regras de rotulagem por janela (as mesmas do dataset, aplicadas a janelas menores)."""
    x = np.minimum(janela, limite)
    if classe == "aproximando":
        return x[:3].mean() - x[-3:].mean() >= QUEDA_MIN_APROX_CM and x[-3:].mean() <= limite - 5
    if classe == "livre":
        return janela.min() >= 40
    return janela.max() <= 22  # obstaculo


def montar(series: list[Serie], n: int, limite: float, modo: str):
    X, y, grupo, origem = [], [], [], []
    descartadas = {c: 0 for c in CLASSES}
    for g, s in enumerate(series):
        for i in range(len(s.dist) - n + 1):
            janela = s.dist[i:i + n]
            if not janela_valida(s.classe, janela, limite):
                descartadas[s.classe] += 1
                continue
            X.append(atributos(janela, limite) if modo == "atributos" else list(np.minimum(janela, limite) / limite))
            y.append(CLASSES.index(s.classe))
            grupo.append(g)
            origem.append((s.arquivo, i))
    return np.array(X), np.array(y), np.array(grupo), origem, descartadas


def novo_modelo() -> LogisticRegression:
    return LogisticRegression(max_iter=5000, class_weight="balanced")


def deixar_um_arquivo_fora(X, y, grupo):
    """Cada arquivo é testado por um modelo que nunca viu nenhuma janela dele."""
    pred = np.zeros_like(y)
    for g in np.unique(grupo):
        treino, teste = grupo != g, grupo == g
        escala = StandardScaler().fit(X[treino])
        modelo = novo_modelo().fit(escala.transform(X[treino]), y[treino])
        pred[teste] = modelo.predict(escala.transform(X[teste]))
    return pred


def comparar(series):
    linhas = []
    for nome, n, limite, modo in [
        ("Valores brutos, 2 s (formato da etapa anterior)", 20, 200.0, "bruto"),
        ("Valores brutos, 1,5 s, limite 100 cm", 15, 100.0, "bruto"),
        ("Atributos de tendência, 1 s, limite 100 cm", 10, 100.0, "atributos"),
        ("Atributos de tendência, 2 s, limite 100 cm", 20, 100.0, "atributos"),
        ("**Atributos de tendência, 1,5 s, limite 100 cm (escolhida)**", 15, 100.0, "atributos"),
    ]:
        X, y, grupo, _, _ = montar(series, n, limite, modo)
        pred = deixar_um_arquivo_fora(X, y, grupo)
        _, rec, _, _ = precision_recall_fscore_support(y, pred, labels=[0, 1, 2], zero_division=0)
        linhas.append({
            "configuracao": nome, "janelas": len(y), "janelas_aproximando": int((y == 1).sum()),
            "acuracia": accuracy_score(y, pred), "f1_macro": f1_score(y, pred, average="macro"),
            "recall": dict(zip(CLASSES, rec.tolist())),
        })
    return linhas


def salvar_comparacao(linhas):
    texto = ["# Configurações testadas", "",
             "Validação deixando um arquivo de fora (15 rodadas). Mesmo modelo (regressão logística), "
             "mudando só a janela e a entrada.", "",
             "| Configuração | Janelas | Janelas `aproximando` | Acurácia | F1 macro | Recall livre | Recall aproximando | Recall obstáculo |",
             "|---|---:|---:|---:|---:|---:|---:|---:|"]
    def br(v, casas):
        return f"{v:.{casas}f}".replace(".", ",")

    for l in linhas:
        r = l["recall"]
        texto.append(f"| {l['configuracao']} | {l['janelas']} | {l['janelas_aproximando']} | {br(100 * l['acuracia'], 1)} % | "
                     f"{br(l['f1_macro'], 3)} | {r['livre']:.0%} | {r['aproximando']:.0%} | {r['obstaculo']:.0%} |")
    texto += ["", "Gerado por `ia/treinar.py`."]
    (RESULTADOS / "comparacao.md").write_text("\n".join(texto) + "\n", encoding="utf-8")


def salvar_matriz(cm):
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    fig, ax = plt.subplots(figsize=(4.6, 3.9), dpi=150)
    ax.imshow(cm / cm.sum(1, keepdims=True), cmap="Blues", vmin=0, vmax=1)
    for i in range(3):
        for j in range(3):
            frac = cm[i, j] / cm[i].sum()
            ax.text(j, i, f"{cm[i, j]}\n{frac:.0%}", ha="center", va="center",
                    color="white" if frac > 0.6 else "#1b1b1b", fontsize=9)
    ax.set_xticks(range(3), CLASSES)
    ax.set_yticks(range(3), CLASSES)
    ax.set_xlabel("previsto")
    ax.set_ylabel("real")
    ax.set_title("Deixando um arquivo de fora", fontsize=10)
    fig.tight_layout()
    fig.savefig(RESULTADOS / "matriz_confusao.png")
    plt.close(fig)


def exportar_cabecalho(escala: StandardScaler, modelo: LogisticRegression):
    def vetor(v):
        return "{" + ", ".join(f"{x:.8g}f" for x in v) + "}"

    pesos = ",\n    ".join(vetor(linha) for linha in modelo.coef_)
    texto = f"""// Gerado por ia/treinar.py. Não edite à mão: rode o script de novo.
// Regressão logística multinomial sobre {len(ATRIBUTOS)} atributos de uma janela de {JANELA} leituras (10 Hz).
#pragma once

namespace modelo {{

constexpr int N_JANELA = {JANELA};
constexpr int N_ATRIBUTOS = {len(ATRIBUTOS)};
constexpr int N_CLASSES = {len(CLASSES)};
constexpr float LIMITE_CM = {LIMITE_CM:.1f}f;
constexpr float PERIODO_S = {PERIODO_S:.1f}f;
constexpr float SEM_ECO_CM = {SEM_ECO_CM:.1f}f;

// Ordem: {", ".join(CLASSES)}
constexpr const char* CLASSES[N_CLASSES] = {{{", ".join(f'"{c}"' for c in CLASSES)}}};

// Ordem: {", ".join(ATRIBUTOS)}
constexpr float MEDIA[N_ATRIBUTOS] = {vetor(escala.mean_)};
constexpr float ESCALA[N_ATRIBUTOS] = {vetor(escala.scale_)};

constexpr float PESOS[N_CLASSES][N_ATRIBUTOS] = {{
    {pesos}
}};
constexpr float VIES[N_CLASSES] = {vetor(modelo.intercept_)};

}}  // namespace modelo
"""
    CABECALHO.write_text(texto, encoding="utf-8")


def main():
    RESULTADOS.mkdir(parents=True, exist_ok=True)
    series = carregar()

    comparacao = comparar(series)
    salvar_comparacao(comparacao)

    X, y, grupo, origem, descartadas = montar(series, JANELA, LIMITE_CM, "atributos")
    pred = deixar_um_arquivo_fora(X, y, grupo)
    cm = confusion_matrix(y, pred, labels=[0, 1, 2])
    prec, rec, f1, sup = precision_recall_fscore_support(y, pred, labels=[0, 1, 2], zero_division=0)
    salvar_matriz(cm)

    escala = StandardScaler().fit(X)
    modelo = novo_modelo().fit(escala.transform(X), y)
    exportar_cabecalho(escala, modelo)

    prob = modelo.predict_proba(escala.transform(X))
    with open(RESULTADOS / "paridade.csv", "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["arquivo", "inicio", *ATRIBUTOS, *(f"p_{c}" for c in CLASSES)])
        for (arq, ini), xs, ps in zip(origem, X, prob):
            w.writerow([arq, ini, *(f"{v:.6f}" for v in xs), *(f"{p:.6f}" for p in ps)])

    metricas = {
        "janela_amostras": JANELA, "janela_s": JANELA * PERIODO_S, "limite_cm": LIMITE_CM,
        "atributos": ATRIBUTOS, "classes": CLASSES,
        "arquivos": len(series), "amostras": int(sum(len(s.dist) for s in series)),
        "janelas_por_classe": {c: int((y == i).sum()) for i, c in enumerate(CLASSES)},
        "janelas_descartadas_pela_regra": descartadas,
        "validacao": "deixando um arquivo de fora",
        "acuracia": round(float(accuracy_score(y, pred)), 4),
        "f1_macro": round(float(f1_score(y, pred, average="macro")), 4),
        "por_classe": {c: {"precisao": round(float(prec[i]), 4), "recall": round(float(rec[i]), 4),
                           "f1": round(float(f1[i]), 4), "janelas": int(sup[i])} for i, c in enumerate(CLASSES)},
        "matriz_confusao": {"linhas_real_colunas_previsto": CLASSES, "valores": cm.tolist()},
        "parametros_modelo": int(modelo.coef_.size + modelo.intercept_.size),
        "comparacao": comparacao,
    }
    (RESULTADOS / "metricas.json").write_text(json.dumps(metricas, ensure_ascii=False, indent=2), encoding="utf-8")

    print(f"{len(y)} janelas {metricas['janelas_por_classe']} | descartadas {descartadas}")
    print(f"acurácia {metricas['acuracia']:.3f} | F1 macro {metricas['f1_macro']:.3f}")
    for c, m in metricas["por_classe"].items():
        print(f"  {c:12s} precisão {m['precisao']:.2f} recall {m['recall']:.2f} F1 {m['f1']:.2f} ({m['janelas']} janelas)")
    print("matriz (real x previsto):", cm.tolist())
    print(f"modelo exportado em {CABECALHO.relative_to(RAIZ)} ({metricas['parametros_modelo']} parâmetros)")


if __name__ == "__main__":
    main()
