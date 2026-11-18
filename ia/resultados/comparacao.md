# Configurações testadas

Validação deixando um arquivo de fora (15 rodadas). Mesmo modelo (regressão logística), mudando só a janela e a entrada.

| Configuração | Janelas | Janelas `aproximando` | Acurácia | F1 macro | Recall livre | Recall aproximando | Recall obstáculo |
|---|---:|---:|---:|---:|---:|---:|---:|
| Valores brutos, 2 s (formato da etapa anterior) | 320 | 10 | 96,2 % | 0,834 | 88% | 80% | 100% |
| Valores brutos, 1,5 s, limite 100 cm | 394 | 34 | 97,2 % | 0,928 | 97% | 76% | 100% |
| Atributos de tendência, 1 s, limite 100 cm | 460 | 50 | 96,3 % | 0,921 | 95% | 80% | 100% |
| Atributos de tendência, 2 s, limite 100 cm | 320 | 10 | 100,0 % | 1,000 | 100% | 100% | 100% |
| **Atributos de tendência, 1,5 s, limite 100 cm (escolhida)** | 394 | 34 | 99,0 % | 0,974 | 98% | 94% | 100% |

Gerado por `ia/treinar.py`.
