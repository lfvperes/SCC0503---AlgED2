/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef REGISTRO_CABECALHO
#define REGISTRO_CABECALHO

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// constantes
#define TAM_REG_CABECALHO 17
#define TAM_REG_DADOS 80
#define OFFSET_REMOVIDO          0 
#define OFFSET_PROXIMO           1
#define OFFSET_COD_ESTACAO       5
#define OFFSET_COD_LINHA         9
#define OFFSET_COD_PROX_ESTACAO  13
#define OFFSET_DIST_PROX_ESTACAO 17
#define OFFSET_COD_LINHA_INTEGRA 21
#define OFFSET_COD_EST_INTEGRA   25
#define OFFSET_TAM_NOME_ESTACAO  29
#define OFFSET_NOME_ESTACAO      33

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

typedef enum {
    CAMPO_REMOVIDO,
    CAMPO_PROXIMO,
    CAMPO_COD_ESTACAO,
    CAMPO_COD_LINHA,
    CAMPO_COD_PROX_ESTACAO,
    CAMPO_DIST_PROX_ESTACAO,
    CAMPO_COD_LINHA_INTEGRA,
    CAMPO_COD_EST_INTEGRA,
    CAMPO_TAM_NOME_ESTACAO,
    CAMPO_NOME_ESTACAO,
    CAMPO_TAM_NOME_LINHA,
    CAMPO_NOME_LINHA
} CampoRegistro;


#endif // REGISTRO_CABECALHO