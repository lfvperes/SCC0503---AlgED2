/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdio.h>
#include <stdlib.h>
#include "arquivo_bin.h"
#include "registro_io.h"
#include "fornecidas.h"
#include "criaIndice.h"
#include "buscaIndexada.h"
#include "escrita.h"

// carrega os pares do arquivo de índice em memória e retorna o total.
// o status já foi validado pelo chamador. o cursor é posicionado logo
// após o byte de status antes de chamar esta função.
// *pares deve ser liberado pelo chamador.
static int carregaPares(FILE *fpIndice, ParIndice **pares) {
    fseek(fpIndice, 0, SEEK_END);
    long tamArquivo = ftell(fpIndice);
    int total = (int)((tamArquivo - sizeof(char)) / (2 * sizeof(int)));

    *pares = malloc(total * sizeof(ParIndice));
    if (*pares == NULL)
        return -1;

    fseek(fpIndice, sizeof(char), SEEK_SET);
    for (int i = 0; i < total; i++) {
        fread(&(*pares)[i].codEstacao, sizeof(int), 1, fpIndice);
        fread(&(*pares)[i].rrn,        sizeof(int), 1, fpIndice);
    }
    return total;
}

// grava o array de pares inteiro no arquivo de índice (sobrescreve).
// preserva o byte de status '1' no início.
static int salvaPares(char *arquivoIndice, ParIndice *pares, int total) {
    FILE *fp = fopen(arquivoIndice, "wb");
    if (fp == NULL)
        return 1;

    char statusIndice = '1';
    fwrite(&statusIndice, sizeof(char), 1, fp);

    for (int i = 0; i < total; i++) {
        fwrite(&pares[i].codEstacao, sizeof(int), 1, fp);
        fwrite(&pares[i].rrn,        sizeof(int), 1, fp);
    }

    fclose(fp);
    return 0;
}

int insertRegistro(char *arquivoEntrada, char *arquivoIndice, int n) {
    // --- abre e valida o arquivo de índice ---
    FILE *fpIndice = fopen(arquivoIndice, "rb");
    if (fpIndice == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return 1;
    }

    char statusIndice;
    if (fread(&statusIndice, sizeof(char), 1, fpIndice) != 1 || statusIndice != '1') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fpIndice);
        return 1;
    }

    // carrega os pares do índice em memória
    ParIndice *pares = NULL;
    int total = carregaPares(fpIndice, &pares);
    fclose(fpIndice);

    if (total < 0) {
        printf("Falha no processamento do arquivo.\n");
        return 1;
    }

    // --- abre e valida o arquivo de dados ---
    FILE *fpDados = fopen(arquivoEntrada, "r+b");
    if (fpDados == NULL) {
        printf("Falha no processamento do arquivo.\n");
        free(pares);
        return 1;
    }

    char status;
    int topo, proxRRN, nroEstacoes, nroParesEstacao;
    if (leCabecalho(fpDados, &status, &topo, &proxRRN, &nroEstacoes, &nroParesEstacao) != 0
        || status != '1') {
        printf("Falha no processamento do arquivo.\n");
        free(pares);
        fclose(fpDados);
        return 1;
    }

    // --- lê os campos do novo registro da entrada padrão ---
    // formato: codEstacao nomeEstacao codLinha nomeLinha
    //          codProxEstacao distProxEstacao codLinhaIntegra codEstIntegra
    // campos inteiros ausentes entram como -1; strings ausentes como ""
    struct registro novo;
    char bufStr[TAM_REG_DADOS];

    for (int i = 0; i < n; i++) {
        // --- leitura dos campos ---
        scanf("%d", &novo.codEstacao);

        ScanQuoteString(bufStr);
        novo.tamNomeEstacao = strlen(bufStr);
        novo.nomeEstacao = malloc(novo.tamNomeEstacao + 1);
        memcpy(novo.nomeEstacao, bufStr, novo.tamNomeEstacao + 1);

        scanf("%d", &novo.codLinha);

        ScanQuoteString(bufStr);
        novo.tamNomeLinha = strlen(bufStr);
        novo.nomeLinha = malloc(novo.tamNomeLinha + 1);
        memcpy(novo.nomeLinha, bufStr, novo.tamNomeLinha + 1);

        char bufInt[32];
        scanf("%s", bufInt);
        novo.codProxEstacao = (strcmp(bufInt, "NULO") == 0) ? -1 : atoi(bufInt);

        scanf("%s", bufInt);
        novo.distProxEstacao = (strcmp(bufInt, "NULO") == 0) ? -1 : atoi(bufInt);

        scanf("%s", bufInt);
        novo.codLinhaIntegra = (strcmp(bufInt, "NULO") == 0) ? -1 : atoi(bufInt);

        scanf("%s", bufInt);
        novo.codEstIntegra = (strcmp(bufInt, "NULO") == 0) ? -1 : atoi(bufInt);

        novo.removido = '0';
        novo.proximo  = -1;

        // --- verifica duplicata ---
        if (buscaBinariaIndice(pares, total, novo.codEstacao) != -1) {
            printf("Chave já existente: registro com codEstacao %d não inserido.\n",
                novo.codEstacao);
            free(novo.nomeEstacao);
            free(novo.nomeLinha);
            continue;
        }

        // --- marca inconsistente ---
        escreveCabecalho(fpDados, '0', topo, proxRRN, nroEstacoes, nroParesEstacao);

        // --- determina onde escrever ---
        int novoRRN;
        if (topo != -1) {
            novoRRN = topo;
            long offsetTopo = TAM_REG_CABECALHO + topo * (long)TAM_REG_DADOS;
            struct registro regRemovido = leRegistro(fpDados, offsetTopo);
            topo = regRemovido.proximo;
            free(regRemovido.nomeEstacao);
            free(regRemovido.nomeLinha);
            fseek(fpDados, offsetTopo, SEEK_SET);
            escreveRegistroDados(fpDados, &novo);
        } else {
            novoRRN = proxRRN;
            fseek(fpDados, 0, SEEK_END);
            escreveRegistroDados(fpDados, &novo);
            proxRRN++;
        }

        // --- atualiza contagens ---
        if (novo.codProxEstacao != -1)
            nroParesEstacao++;

        int nomeJaExiste = 0;
        for (int k = 0; k < proxRRN; k++) {
            if (k == novoRRN) continue;
            long offK = TAM_REG_CABECALHO + k * (long)TAM_REG_DADOS;
            struct registro regK = leRegistro(fpDados, offK);
            if (regK.removido == '0' &&
                regK.tamNomeEstacao == novo.tamNomeEstacao &&
                memcmp(regK.nomeEstacao, novo.nomeEstacao, novo.tamNomeEstacao) == 0) {
                nomeJaExiste = 1;
                free(regK.nomeEstacao);
                free(regK.nomeLinha);
                break;
            }
            free(regK.nomeEstacao);
            free(regK.nomeLinha);
        }
        if (!nomeJaExiste)
            nroEstacoes++;

        // --- atualiza cabeçalho ---
        escreveCabecalho(fpDados, '1', topo, proxRRN, nroEstacoes, nroParesEstacao);

        // --- atualiza índice em memória ---
        pares = realloc(pares, (total + 1) * sizeof(ParIndice));
        int posInsert = total;
        for (int k = 0; k < total; k++) {
            if (pares[k].codEstacao > novo.codEstacao) {
                posInsert = k;
                break;
            }
        }
        for (int k = total; k > posInsert; k--)
            pares[k] = pares[k - 1];
        pares[posInsert].codEstacao = novo.codEstacao;
        pares[posInsert].rrn        = novoRRN;
        total++;

        free(novo.nomeEstacao);
        free(novo.nomeLinha);
    }

    fclose(fpDados);
    free(pares);

    BinarioNaTela(arquivoEntrada);
    criaIndice(arquivoEntrada, arquivoIndice);
    return 0;
}