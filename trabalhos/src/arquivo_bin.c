/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdio.h>
#include <string.h>
#include "registro.h"
#include "arquivo_bin.h"

// offsets internos do registro de cabeçalho
#define OFF_STATUS          0
#define OFF_TOPO            (sizeof(char))
#define OFF_PROX_RRN        (OFF_TOPO + sizeof(int))
#define OFF_NRO_ESTACOES    (OFF_TOPO + 2 * sizeof(int))
#define OFF_NRO_PARES       (OFF_TOPO + 3 * sizeof(int))

// inicializa ou atualiza o registro de cabeçalho no arquivo binário.
// sempre posiciona no início antes de escrever.
// retorna 0 em sucesso, 1 em erro.
int escreveCabecalho(FILE *fpBin, char status, int proxRRN,
                     int nroEstacoes, int nroParesEstacao) {
    char buf[TAM_REG_CABECALHO];
    memset(buf, 0, TAM_REG_CABECALHO);

    *(char *)(buf + OFF_STATUS)       = status;
    *(int  *)(buf + OFF_TOPO)         = -1;          // topo da lista de removidos
    *(int  *)(buf + OFF_PROX_RRN)     = proxRRN;
    *(int  *)(buf + OFF_NRO_ESTACOES) = nroEstacoes;
    *(int  *)(buf + OFF_NRO_PARES)    = nroParesEstacao;

    fseek(fpBin, 0, SEEK_SET);
    if (fwrite(buf, TAM_REG_CABECALHO, 1, fpBin) != 1) {
        perror("Erro ao escrever cabecalho no arquivo binario");
        return 1;
    }
    fseek(fpBin, 0, SEEK_END);
    return 0;
}

// lê o registro de cabeçalho e preenche os ponteiros fornecidos.
// sempre posiciona no início antes de ler.
// retorna 0 em sucesso, 1 em erro.
int leCabecalho(FILE *fpBin, char *status, int *topo,
                int *proxRRN, int *nroEstacoes, int *nroParesEstacao) {
    char buf[TAM_REG_CABECALHO];

    fseek(fpBin, 0, SEEK_SET);
    if (fread(buf, TAM_REG_CABECALHO, 1, fpBin) != 1) {
        perror("Erro ao ler cabecalho do arquivo binario");
        return 1;
    }

    *status          = *(char *)(buf + OFF_STATUS);
    *topo            = *(int  *)(buf + OFF_TOPO);
    *proxRRN         = *(int  *)(buf + OFF_PROX_RRN);
    *nroEstacoes     = *(int  *)(buf + OFF_NRO_ESTACOES);
    *nroParesEstacao = *(int  *)(buf + OFF_NRO_PARES);

    return 0;
}
