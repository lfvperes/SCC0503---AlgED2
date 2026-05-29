/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registro.h"
#include "registro_io.h"
#include "arquivo_bin.h"
#include "filtro.h"
#include "fornecidas.h"
#include "leitura.h"

#define MAX_LINHA 1024

// lê arquivo binário e imprime registros de dados formatados
void imprimeTabela(char *arquivoEntrada) {
    FILE *fpBin = fopen(arquivoEntrada, "rb");

    if (fpBin == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }

    char status;
    int topo, proxRRN, nroEstacoes, nroParesEstacao;
    leCabecalho(fpBin, &status, &topo, &proxRRN, &nroEstacoes, &nroParesEstacao);

    if (status == '0') {
        printf("Falha no processamento do arquivo.");
        fclose(fpBin);
        return;
    }

    for (int i = 0; i < proxRRN; i++) {
        int offset = TAM_REG_CABECALHO + i * TAM_REG_DADOS;
        struct registro dados = leRegistro(fpBin, offset);

        if (dados.removido == '1') {
            free(dados.nomeEstacao);
            free(dados.nomeLinha);
            continue;
        }

        imprimeRegistro(dados);
        free(dados.nomeEstacao);
        free(dados.nomeLinha);
    }

    fclose(fpBin);
}

int listaTabelaFiltro(char *arquivoEntrada, int n) {
    char bufferLinha[MAX_LINHA];
    int m, encontrados, filtro = 1;
    FILE *fpBin = fopen(arquivoEntrada, "rb");

    if (fpBin == NULL) {
        printf("Falha no processamento do arquivo.");
        return 1;
    }

    char status;
    int topo, proxRRN, nroEstacoes, nroParesEstacao;
    leCabecalho(fpBin, &status, &topo, &proxRRN, &nroEstacoes, &nroParesEstacao);

    if (status == '0') {
        printf("Falha no processamento do arquivo.");
        fclose(fpBin);
        return 1;
    }

    if (n == 0) {
        filtro = 0;
        n = 1;
    }

    for (int i = 0; i < n; i++) {

        if (filtro) {
            scanf("%d", &m);
        } else {
            m = 0;
        }

        char **nomeCampo  = malloc(m * sizeof(char *));
        char **valorCampo = malloc(m * sizeof(char *));

        for (int j = 0; j < m; j++) {
            scanf("%s", bufferLinha);
            nomeCampo[j] = malloc(strlen(bufferLinha) + 1);
            strcpy(nomeCampo[j], bufferLinha);

            if (strcmp(nomeCampo[j], "nomeEstacao") == 0 ||
                strcmp(nomeCampo[j], "nomeLinha") == 0) {
                ScanQuoteString(bufferLinha);
            } else {
                scanf("%s", bufferLinha);
            }

            valorCampo[j] = malloc(strlen(bufferLinha) + 1);
            strcpy(valorCampo[j], bufferLinha);
        }

        struct registro *resultado = buscaRegistros(fpBin, nomeCampo, valorCampo,
                                                    m, proxRRN, &encontrados);

        if (resultado == NULL && encontrados == 0) {
            printf("Registro inexistente.\n");
        } else if (resultado == NULL) {
            printf("Falha no processamento do arquivo.\n");
        } else {
            for (int j = 0; j < encontrados; j++) {
                imprimeRegistro(resultado[j]);
                free(resultado[j].nomeEstacao);
                free(resultado[j].nomeLinha);
            }
            free(resultado);
        }

        for (int j = 0; j < m; j++) {
            free(nomeCampo[j]);
            free(valorCampo[j]);
        }
        free(nomeCampo);
        free(valorCampo);

        if (i < n - 1)
            printf("\n");
    }

    fclose(fpBin);
    return 0;
}

int acessoRRN(char *arquivoEntrada, int RRN) {
    FILE *fpBin = fopen(arquivoEntrada, "rb");

    if (fpBin == NULL) {
        printf("Falha no processamento do arquivo.");
        return 1;
    }

    char status;
    int topo, proxRRN, nroEstacoes, nroParesEstacao;
    leCabecalho(fpBin, &status, &topo, &proxRRN, &nroEstacoes, &nroParesEstacao);

    if (status == '0') {
        printf("Falha no processamento do arquivo.");
        fclose(fpBin);
        return 1;
    }

    if (proxRRN < RRN) {
        printf("Registro inexistente.");
        fclose(fpBin);
        return 1;
    }

    int offset = TAM_REG_CABECALHO + RRN * TAM_REG_DADOS;
    struct registro dados = leRegistro(fpBin, offset);

    if (dados.removido == '1') {
        free(dados.nomeEstacao);
        free(dados.nomeLinha);
        printf("Registro inexistente.");
        fclose(fpBin);
        return 1;
    }

    imprimeRegistro(dados);
    free(dados.nomeEstacao);
    free(dados.nomeLinha);
    fclose(fpBin);
    return 0;
}
