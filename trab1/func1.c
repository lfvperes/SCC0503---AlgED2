#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registro.h"

char bufferCabecalho[TAM_REG_CABECALHO];

static void escreveRegistroDados(FILE *fpBin, const struct registro *dados) {

    char bufferDados[TAM_REG_DADOS];
    memset(bufferDados, 0, TAM_REG_DADOS);

    size_t offset = 0;

    // removido
    memcpy(bufferDados + offset, &dados->removido, sizeof(dados->removido));
    offset += sizeof(dados->removido);

    // proximo
    memcpy(bufferDados + offset, &dados->proximo, sizeof(dados->proximo));
    offset += sizeof(dados->proximo);

    // codEstacao
    memcpy(bufferDados + offset, &dados->codEstacao, sizeof(dados->codEstacao));
    offset += sizeof(dados->codEstacao);

    // codLinha
    memcpy(bufferDados + offset, &dados->codLinha, sizeof(dados->codLinha));
    offset += sizeof(dados->codLinha);

    // codProxEstacao
    memcpy(bufferDados + offset, &dados->codProxEstacao, sizeof(dados->codProxEstacao));
    offset += sizeof(dados->codProxEstacao);

    // distProxEstacao
    memcpy(bufferDados + offset, &dados->distProxEstacao, sizeof(dados->distProxEstacao));
    offset += sizeof(dados->distProxEstacao);

    // codLinhaIntegra
    memcpy(bufferDados + offset, &dados->codLinhaIntegra, sizeof(dados->codLinhaIntegra));
    offset += sizeof(dados->codLinhaIntegra);

    // codEstIntegra
    memcpy(bufferDados + offset, &dados->codEstIntegra, sizeof(dados->codEstIntegra));
    offset += sizeof(dados->codEstIntegra);

    // tamNomeEstacao
    memcpy(bufferDados + offset, &dados->tamNomeEstacao, sizeof(dados->tamNomeEstacao));
    offset += sizeof(dados->tamNomeEstacao);

    memcpy(bufferDados + offset, dados->nomeEstacao, dados->tamNomeEstacao);
    offset += dados->tamNomeEstacao;

    // tamNomeLinha
    memcpy(bufferDados + offset, &dados->tamNomeLinha, sizeof(dados->tamNomeLinha));
    offset += sizeof(dados->tamNomeLinha);

    // nomeLinha - só copia se tam > 0 
    if (dados->tamNomeLinha > 0)
        memcpy(bufferDados + offset, dados->nomeLinha, dados->tamNomeLinha);
    offset += dados->tamNomeLinha;

    // preenche lixo
    memset(bufferDados + offset, '$', TAM_REG_DADOS - offset);

    // escreve buffer do registro de dados no arquivo
    if (fwrite(bufferDados, TAM_REG_DADOS, 1, fpBin) != 1) {
        perror("Error writing data record to binary file");
    }
}

static int escreveRegistroCabecalho(FILE *fpBin) {
    // offsets dos campos de cabecalho
    size_t offset_status = 0;
    size_t offset_topo = 1;
    size_t offset_proxRRN = 1 + sizeof(int);              // 5
    size_t offset_nroEstacoes = 1 + 2 * sizeof(int);      // 9
    size_t offset_nroParesEstacao = 1 + 3 * sizeof(int);  // 13

    // inicializando cabecalho
    memset(bufferCabecalho, 0, TAM_REG_CABECALHO);
    *(char *)(bufferCabecalho + offset_status) = '0';
    *(int *)(bufferCabecalho + offset_topo) = -1;
    *(int *)(bufferCabecalho + offset_proxRRN) = 0;
    *(int *)(bufferCabecalho + offset_nroEstacoes) = 0;
    *(int *)(bufferCabecalho + offset_nroParesEstacao) = 0;

    // escreve cabecalho no arquivo
    if (fwrite(bufferCabecalho, TAM_REG_CABECALHO, 1, fpBin) != 1) {
        perror("Erro ao escrever cabecalho no arquivo binario");
        fclose(fpBin);
        return 1;
    }
    return 0;
}

static void lerRegistroCSV(char *linha, struct registro *dados) {
    char *ptr = linha;
        char *token;

        // codEstacao - não aceita nulo
        token = strsep(&ptr, ",");
        dados->codEstacao = atoi(token);

        // nomeEstacao - não aceita nulo
        token = strsep(&ptr, ",");
        dados->tamNomeEstacao = strlen(token);
        dados->nomeEstacao = malloc(dados->tamNomeEstacao);
        memcpy(dados->nomeEstacao, token, dados->tamNomeEstacao);

        // codLinha - nulo representado por -1
        token = strsep(&ptr, ",");
        dados->codLinha = (strlen(token) == 0) ? -1 : atoi(token);

        // nomeLinha - nulo representado por tamNomeLinha = 0
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
        token = strsep(&ptr, ",");
        dados->codEstIntegra = (strlen(token) == 0) ? -1 : atoi(token);

        dados->removido = '0';
        dados->proximo = -1;

        return;
}

int func1(char *estacoesCSV, char *estacoesBin) {
    FILE *fpCSV = fopen(estacoesCSV, "r");
    FILE *fpBin = fopen(estacoesBin, "wb");

    struct registro dados;

    if (fpBin == NULL) {
        perror("Erro ao abrir arquivo binario para escrita");
        return 1;
    }

    escreveRegistroCabecalho(fpBin);

    char linha[TAM_REG_DADOS];

    // pula o cabeçalho do CSV, linha inteira independente do tamanho
    int c;
    while ((c = fgetc(fpCSV)) != '\n' && c != EOF);

    while (fgets(linha, TAM_REG_DADOS, fpCSV)) {

        lerRegistroCSV(linha, &dados);

        escreveRegistroDados(fpBin, &dados);

        free(dados.nomeEstacao);
        free(dados.nomeLinha);
    }

    fclose(fpCSV);
    fclose(fpBin);
    return 0;
}