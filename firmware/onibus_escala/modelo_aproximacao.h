// Gerado por ia/treinar.py. Não edite à mão: rode o script de novo.
// Regressão logística multinomial sobre 7 atributos de uma janela de 15 leituras (10 Hz).
#pragma once

namespace modelo {

constexpr int N_JANELA = 15;
constexpr int N_ATRIBUTOS = 7;
constexpr int N_CLASSES = 3;
constexpr float LIMITE_CM = 100.0f;
constexpr float PERIODO_S = 0.1f;
constexpr float SEM_ECO_CM = 200.0f;

// Ordem: livre, aproximando, obstaculo
constexpr const char* CLASSES[N_CLASSES] = {"livre", "aproximando", "obstaculo"};

// Ordem: ultimo, media, minimo, desvio, inclinacao, queda, sem_eco
constexpr float MEDIA[N_ATRIBUTOS] = {35.091371f, 35.47934f, 31.120305f, 3.4880252f, -0.56571791f, 0.68764805f, 0.15939086f};
constexpr float ESCALA[N_ATRIBUTOS] = {33.613304f, 31.612386f, 29.119817f, 7.1542535f, 14.053888f, 17.316523f, 0.34056135f};

constexpr float PESOS[N_CLASSES][N_ATRIBUTOS] = {
    {1.0402884f, 1.0627415f, 0.8001671f, 0.84651452f, 0.92747073f, -0.70417277f, 1.3790086f},
    {0.059781243f, 0.25302092f, 0.31816227f, 0.43964546f, -1.8807245f, 1.6349846f, -1.3816415f},
    {-1.1000696f, -1.3157624f, -1.1183294f, -1.28616f, 0.9532538f, -0.93081183f, 0.002632923f}
};
constexpr float VIES[N_CLASSES] = {1.0587969f, -1.5279439f, 0.46914699f};

}  // namespace modelo
