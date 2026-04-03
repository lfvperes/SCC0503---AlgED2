#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINHA 1024 // numero maximo de caracteres na entrada
#define MAX_ITENS 100
int listaTabelaFiltro(char *arquivoEntrada, int n) {
    char linha[MAX_LINHA];    // buffer para a entrada
    FILE *fpBin = fopen(arquivoEntrada, "rb");    

    for (int i = 0; i < n; i++) {
        
        // lê linha inteira do stdin
        if (!fgets(linha, sizeof(linha), stdin)) {
            printf("Falha no processamento do arquivo.");
            return 1;
        }
        
        // remove "\r\n" que fgets deixa no final da string
        linha[strcspn(linha, "\r\n")] = '\0';

        // primeiro token: numero m
        char *token = strtok(linha, " ");
        if (!token) {
            printf("Falha no processamento do arquivo.");
            return 1;
        }

        // converte m para inteiro
        int m = (int)strtol(token, NULL, 10);

        // aloca m ponteiros para strings (os nomes)
        char **nomeCampo = malloc(m * sizeof(char *));

        // aloca m strings para os valores (tamanho fixo por enquanto)
        char **valorCampo = malloc(m * sizeof(char *));

        // verifica se as alocações funcionaram
        if (!nomeCampo || !valorCampo) {
            printf("Falha na alocação de memória.\n");
            return 1;
        }
        
    }

    fclose(fpBin);
    return 0;
}