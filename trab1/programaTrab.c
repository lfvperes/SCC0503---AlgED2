#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registro.h"

#define TAM_REG_CABECALHO 17
#define TAM_REG_DADOS 80

// variavel global do cabecalho como array 
char bufferCabecalho[TAM_REG_CABECALHO];

int func1(char *estacoesCSV, char *estacoesBin);
int func2(char *arquivoEntrada);
int func3(char *arquivoEntrada, char *n);
int func4(char *arquivoEntrada, char *RRN);

int main(int argc, char *argv[]) {

    int funcionalidade = atoi(argv[1]);
    switch(funcionalidade) {
        case 1:
            func1(argv[2], argv[3]);
            break;
        case 2:
            func2(argv[2]);
            break;
        case 3:
            func3(argv[2], argv[3]);
            break;
        case 4:
            func4(argv[2], argv[3]);
            break;
    }

    return 0;
}

int func1(char *estacoesCSV, char *estacoesBin) {
    FILE *fpCSV = fopen(estacoesCSV, "r");
    // abre arquivo para escrita em binario
    FILE *fpBin = fopen(estacoesBin, "wb");

    char bufferDados[TAM_REG_DADOS];
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
    while (fgets(linha, TAM_REG_DADOS, fpCSV)) {
        char *token = strtok(linha, ",");
        dados.codEstacao = atoi(token);
        
        token = strtok(NULL, ",");
        dados.nomeEstacao = token;
        dados.tamNomeEstacao = strlen(dados.nomeEstacao);
        
        token = strtok(NULL, ",");
        dados.codLinha = atoi(token);
        
        token = strtok(NULL, ",");
        dados.nomeLinha = token;
        dados.tamNomeLinha = strlen(dados.nomeLinha);
        
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
    }

    fclose(fpCSV);
    fclose(fpBin);
    return 0;
}

// Placeholder implementations for other functions
int func2(char *arquivoEntrada) {
    printf("Function 2 called with: %s\n", arquivoEntrada);
    return 0;
}
int func3(char *arquivoEntrada, char *n) {
    printf("Function 3 called with: %s, %s\n", arquivoEntrada, n);
    return 0;
}
int func4(char *arquivoEntrada, char *RRN) {
    printf("Function 4 called with: %s, %s\n", arquivoEntrada, RRN);
    return 0;
}

void escreveRegistroDados(struct registro dados, char *bufferDados) {

    return;
}