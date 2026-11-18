# Roteiro do vídeo (até 6 min)

Filmar o carrinho em um chão liso, com espaço de uns 2 m à frente. O display OLED mostra o estado e a decisão da IA,
então vale enquadrar o display de perto em alguns momentos. Deixar o Monitor Serial aberto num notebook ajuda a
mostrar as probabilidades (`[IA] livre=... aproximando=... obstaculo=...`).

Antes de gravar: letreiro impresso (`cad/letreiro_875A-10.svg`) dentro do suporte, bateria carregada, firmware
com `credenciais.h` preenchido se for mostrar a consulta ao MVP. Para ligar o modo autônomo sem mostrar o celular,
segure o botão BOOT por 1 s (o carrinho dá um bipe curto).

| Tempo | Cena | O que mostrar | O que falar |
|---|---|---|---|
| 0:00 | Abertura | integrantes, nome do produto | Ponto Falante ajuda pessoas com deficiência visual e idosos a pegar ônibus em SP. O carrinho faz o papel do ônibus chegando ao ponto. |
| 0:30 | Hardware | carrinho de perto: HC-SR04, LDR na torre, letreiro, buzzer, display | ESP32, ponte H, sensor ultrassônico, LDR como sensor adicional, peça impressa do letreiro. |
| 1:10 | Caminho livre | modo autônomo, carrinho andando reto; display em SEGUINDO | A IA olha 1,5 s de distâncias e decide a cada 0,2 s. Caminho livre: segue. |
| 1:40 | Alguém se aproxima | com o carrinho andando, aproximar a mão pela frente devagar; bipe e redução; display em REDUZINDO | A IA percebe que a distância está caindo e reduz antes de chegar perto, com aviso sonoro, como o ônibus chegando ao ponto. |
| 2:20 | Obstáculo parado | caixa parada no caminho; carrinho para com bipe duplo; display em PARADO e IA "obstaculo" | Algo parado à frente: para. Também há uma regra fixa que para a 20 cm, mesmo se a IA errar. |
| 2:50 | Retomada | tirar a caixa; carrinho volta a andar em 1 a 2 s | Só retoma quando a IA confirma caminho livre duas vezes. |
| 3:10 | Freio de emergência | colocar a mão de repente a poucos centímetros; alarme triplo | A 8 cm, freio de emergência, independente da IA. |
| 3:40 | Letreiro à noite | apagar a luz ou cobrir o LDR; letreiro acende; voltar a luz; apaga | Com pouca luz o letreiro acende, para continuar legível para quem enxerga pouco e para a conferência por câmera do app. |
| 4:20 | Integração com o MVP | tocar o BOOT; display mostra "PROXIMOS" e a frase dos ônibus do ponto; opcional: mostrar o app no ar | O ESP32 usa o contrato de dispositivos do MVP e recebe a frase com os próximos ônibus do ponto da Paulista. |
| 5:00 | Fechamento | carrinho parado com o letreiro aceso | Sem câmera, sem dado pessoal, IA rodando no próprio ESP32. ODS 11, 10 e 3. |

Depois de gravar: subir no YouTube como não listado, conferir que abre numa janela anônima e colocar o link no
README, na seção "Vídeo de demonstração".
