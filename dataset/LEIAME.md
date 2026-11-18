# Dataset de distâncias do HC-SR04

15 séries gravadas com o sensor ultrassônico frontal deste carrinho na etapa anterior da disciplina (CP2), usadas
para treinar o classificador de `ia/treinar.py`.

## Formato

- Arquivo `classe.N.csv`, colunas `timestamp,dist_cm`.
- `timestamp` em ms, começando em 0, passo de 100 ms (10 Hz).
- `dist_cm` já passou pelo mesmo tratamento que o firmware aplica antes da IA (`filtro_distancia.cpp`):
  sem eco ou 200 cm ou mais vira 200 cm; leitura abaixo de 2 cm repete a última válida; mediana das 3 últimas.
- A classe é o texto antes do primeiro ponto do nome do arquivo.

## Classes

| Classe | Situação gravada | Regra usada para rotular |
|---|---|---|
| `livre` | nada perto ou sem eco | todas as leituras ≥ 40 cm |
| `aproximando` | objeto chegando perto do sensor | queda de 15 cm ou mais ao longo de 2 s, começando em ≥ 25 cm |
| `obstaculo` | objeto parado perto | todas as leituras ≤ 22 cm, pouca variação |

| Classe | Arquivos | Amostras | Duração |
|---|---:|---:|---:|
| livre | 5 | 180 | 18,0 s |
| aproximando | 5 | 105 | 10,5 s |
| obstaculo | 5 | 320 | 32,0 s |
| **Total** | **15** | **605** | **60,5 s** |

## Como o treino usa os arquivos

`ia/treinar.py` corta janelas de 1,5 s (15 leituras), deslizando uma leitura por vez, e aplica de novo a regra de
cada classe à janela menor: uma janela de um arquivo `aproximando` só entra se cair pelo menos 10 cm (sobrou 1
janela fora). O resultado são 394 janelas: 110 `livre`, 34 `aproximando`, 250 `obstaculo`.

## Limitações

- Pouco volume, principalmente de `aproximando` (5 gravações curtas).
- Rótulos por regra de distância e tendência, não anotados à mão durante a gravação.
- Obstáculos entre 7 e 19 cm; faltam outras distâncias, ângulos e materiais (tecido e superfícies inclinadas
  devolvem pouco eco).
- As aproximações foram gravadas com o carrinho parado e o objeto vindo até ele. Com o carrinho andando a
  distância também cai, então o padrão é o mesmo, mas a velocidade de aproximação muda.

## Como gravar mais

No painel do carrinho, em **Coleta de dados**: Gravar, executar a situação, Parar e Baixar CSV. O arquivo sai neste
formato. Renomeie para `classe.N.csv`, coloque em `dataset/csv/` e rode `python ia/treinar.py`.
