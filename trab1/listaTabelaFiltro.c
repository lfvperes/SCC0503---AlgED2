#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINHA 1024 // numero maximo de caracteres na entrada
#define MAX_ITENS 100

void ScanQuoteString(char *str);

int listaTabelaFiltro(char *arquivoEntrada, int n) {
    char bufferLinha[MAX_LINHA];    // buffer para a entrada
    int m;
    FILE *fpBin = fopen(arquivoEntrada, "rb");    
    
    if (fpBin == NULL) {
        printf("Falha no processamento do arquivo.");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        
        // lê m direto do stdin
        scanf("%d", &m);

        // aloca m ponteiros para strings (os nomes)
        char **nomeCampo = malloc(m * sizeof(char *));

        // aloca m ponteiros para os valores
        char **valorCampo = malloc(m * sizeof(char *));

        for(int j = 0; j < m; j++) {
            // lê nome do campos
            scanf("%s", bufferLinha);
            // aloca o necessário e copia
            nomeCampo[i] = malloc(strlen(bufferLinha) + 1);
            strcpy(nomeCampo[i], bufferLinha);

            // lê valor do campo
            ScanQuoteString(bufferLinha);
            // aloca o necessário e copia com +1 para \0
            valorCampo[i] = malloc(strlen(bufferLinha) + 1);
            strcpy(valorCampo[i], bufferLinha);
        }

        // libera strings individuais
        for (int j = 0; j < m; j++) {
            free(nomeCampo[i]);
            free(valorCampo[i]);
        }

        // libera o array de ponteiros
        free(nomeCampo);
        free(valorCampo);        
    }

    fclose(fpBin);
    return 0;
}