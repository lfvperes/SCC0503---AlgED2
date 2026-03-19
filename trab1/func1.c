// func1.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registro.h" // Include the common header

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

    // nomeEstacao
    memcpy(bufferDados + offset, dados->nomeEstacao, dados->tamNomeEstacao);
    offset += dados->tamNomeEstacao;

    // tamNomeLinha
    memcpy(bufferDados + offset, &dados->tamNomeLinha, sizeof(dados->tamNomeLinha));
    offset += sizeof(dados->tamNomeLinha);
    
    // nomeLinha
    memcpy(bufferDados + offset, dados->nomeLinha, dados->tamNomeLinha);
    offset += dados->tamNomeLinha;

    // preenche lixo
    memset(bufferDados + offset, '$', TAM_REG_DADOS - offset);

    // escreve buffer do registro de dados no arquivo
    if (fwrite(bufferDados, TAM_REG_DADOS, 1, fpBin) != 1) {
        perror("Error writing data record to binary file");
        // In a real scenario, you might want to signal an error to the caller
    }
}


int func1(char *estacoesCSV, char *estacoesBin) {
    FILE *fpCSV = fopen(estacoesCSV, "r");
    // abre arquivo para escrita em binario
    FILE *fpBin = fopen(estacoesBin, "wb");

    struct registro dados;


    if (fpBin == NULL) {
        perror("Erro ao abrir arquivo binario para escrita");
        return 1;
    }

    // offsets dos campos de cabecalho
    size_t offset_status = 0;
    size_t offset_topo = 1;
    size_t offset_proxRRN = 1 + sizeof(int); // 5
    size_t offset_nroEstacoes = 1 + 2 * sizeof(int); // 9
    size_t offset_nroParesEstacao = 1 + 3 * sizeof(int); // 13
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

    char linha[TAM_REG_DADOS];
    // pula o cabeçalho do CSV
    fgets(linha, TAM_REG_DADOS, fpCSV);
    while (fgets(linha, TAM_REG_DADOS, fpCSV)) {
        char *token = strtok(linha, ",");
        dados.codEstacao = atoi(token);
        
        token = strtok(NULL, ",");
        dados.tamNomeEstacao = strlen(token);
        dados.nomeEstacao = malloc(dados.tamNomeEstacao);
        memcpy(dados.nomeEstacao, token, dados.tamNomeEstacao);
        
        token = strtok(NULL, ",");
        dados.codLinha = atoi(token);
        
        token = strtok(NULL, ",");
        dados.tamNomeLinha = strlen(token);
        dados.nomeLinha = malloc(dados.tamNomeLinha);
        memcpy(dados.nomeLinha, token, dados.tamNomeLinha);
        
        token = strtok(NULL, ",");
        dados.codProxEstacao = atoi(token);
        
        token = strtok(NULL, ",");
        dados.distProxEstacao = atoi(token);
        
        token = strtok(NULL, ",");
        dados.codLinhaIntegra = atoi(token);
        
        token = strtok(NULL, ",");
        dados.codEstIntegra = atoi(token);
        
        dados.removido = '0';
        dados.proximo = -1;

        escreveRegistroDados(fpBin, &dados);
 
        free(dados.nomeEstacao);
        free(dados.nomeLinha);
    }

    fclose(fpCSV);
    fclose(fpBin);
    return 0;
}