// Copie este arquivo para credenciais.h (que fica fora do Git) e preencha.
// Sem credenciais.h o firmware compila com estes valores: painel local funcionando e MVP desligado.
#pragma once

// Painel do carrinho: rede Wi-Fi criada pelo próprio ESP32 (senha com 8 caracteres ou mais).
#define PAINEL_REDE "PontoFalante-Onibus"
#define PAINEL_SENHA "pontofalante"

// Rede com internet para falar com o MVP (ex.: roteador do celular). Vazio = sem MVP.
#define WIFI_REDE ""
#define WIFI_SENHA ""

// MVP do Ponto Falante. A chave é o valor de DISPOSITIVO_API_KEY nas variáveis de ambiente do Render.
#define MVP_URL "https://ponto-falante.onrender.com"
#define MVP_CHAVE_DISPOSITIVO ""
#define MVP_DISPOSITIVO_ID "esp32-onibus-escala-01"
#define MVP_PONTO 70016916  // Avenida Paulista, 1374 (exemplo público do README do MVP)
