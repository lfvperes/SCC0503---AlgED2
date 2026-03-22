#ifndef REGISTRO_CABECALHO
#define REGISTRO_CABECALHO

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// constantes
#define TAM_REG_CABECALHO 17
#define TAM_REG_DADOS 80

// struct para o registro de dados
struct registro {
    char removido;
    int proximo;
    int codEstacao;
    int codLinha;
    int codProxEstacao;
    int distProxEstacao;
    int codLinhaIntegra;
    int codEstIntegra;
    int tamNomeEstacao;
    char *nomeEstacao;
    int tamNomeLinha;
    char *nomeLinha;
};

// struct para o par e estacoes
struct parEstacao {
    int codEstacao1;
    int codEstacao2;
};

#endif // REGISTRO_CABECALHO