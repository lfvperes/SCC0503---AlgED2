/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registro.h"
#include "csv_parser.h"

// descarta a linha de cabeçalho do CSV caractere a caractere,
// suportando linhas maiores que TAM_REG_DADOS
void descartaCabecalhoCSV(FILE *fp) {
    int c;
    while ((c = fgetc(fp)) != '\n' && c != EOF);
}

// lê uma linha do CSV já carregada em 'linha' e preenche a struct registro.
// campos inteiros vazios são representados por -1.
// campos de string vazios são representados por tamNome = 0.
// aloca memória para os campos de string — deve ser liberada após o uso.
void lerRegistroCSV(char *linha, struct registro *dados) {
    char *ptr = linha;
    char *token;

    // codEstacao - não aceita nulo
    token = strsep(&ptr, ",");
    dados->codEstacao = atoi(token);

    // nomeEstacao - não aceita nulo, aloca e copia o conteúdo do token
    token = strsep(&ptr, ",");
    dados->tamNomeEstacao = strlen(token);
    dados->nomeEstacao = malloc(dados->tamNomeEstacao);
    memcpy(dados->nomeEstacao, token, dados->tamNomeEstacao);

    // codLinha - nulo representado por -1
    token = strsep(&ptr, ",");
    dados->codLinha = (strlen(token) == 0) ? -1 : atoi(token);

    // nomeLinha - nulo representado por tamNomeLinha = 0, aloca e copia se não vazio
    token = strsep(&ptr, ",");
    dados->tamNomeLinha = strlen(token);
    dados->nomeLinha = malloc(dados->tamNomeLinha);
    memcpy(dados->nomeLinha, token, dados->tamNomeLinha);

    // codProxEstacao - nulo representado por -1
    token = strsep(&ptr, ",");
    dados->codProxEstacao = (strlen(token) == 0) ? -1 : atoi(token);

    // distProxEstacao - nulo representado por -1
    token = strsep(&ptr, ",");
    dados->distProxEstacao = (strlen(token) == 0) ? -1 : atoi(token);

    // codLinhaIntegra - nulo representado por -1
    token = strsep(&ptr, ",");
    dados->codLinhaIntegra = (strlen(token) == 0) ? -1 : atoi(token);

    // codEstIntegra - nulo representado por -1
    token = strsep(&ptr, "\n");
    dados->codEstIntegra = (strlen(token) == 0) ? -1 : atoi(token);

    // campos de controle: não removido e sem próximo
    dados->removido = '0';
    dados->proximo = -1;
}
