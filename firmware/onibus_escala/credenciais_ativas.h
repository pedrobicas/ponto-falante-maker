// Usa credenciais.h quando existir; senão, os valores de exemplo (sem MVP).
#pragma once

#if __has_include("credenciais.h")
#include "credenciais.h"
#define CREDENCIAIS_PROPRIAS 1
#else
#include "credenciais.exemplo.h"
#define CREDENCIAIS_PROPRIAS 0
#endif
