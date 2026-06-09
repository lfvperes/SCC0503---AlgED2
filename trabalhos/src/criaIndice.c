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

// comparador para qsort: ordena por codEstacao crescente
int comparaPar(const void *a, const void *b) {
    return ((ParIndice *)a)->codEstacao - ((ParIndice *)b)->codEstacao;
}

int criaIndice(char *arquivoEntrada, char *arquivoIndice) {
    FILE *fpEntrada = fopen(arquivoEntrada, "rb");
    if (fpEntrada == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return 1;
    }

    // lê e valida o cabeçalho do arquivo de dados
    char status;
    int topo, proxRRN, nroEstacoes, nroParesEstacao;
    if (leCabecalho(fpEntrada, &status, &topo, &proxRRN, &nroEstacoes, &nroParesEstacao) != 0
        || status != '1') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fpEntrada);
        return 1;
    }

    // array dinâmico para coletar os pares (codEstacao, RRN)
    int capacidade = 16;
    int total = 0;
    ParIndice *pares = malloc(capacidade * sizeof(ParIndice));
    if (pares == NULL) {
        printf("Falha no processamento do arquivo.\n");
        fclose(fpEntrada);
        return 1;
    }

    // percorre todos os registros sequencialmente, coletando pares de não-removidos
    int rrn = 0;
    struct registro reg;
    while (1) {
        reg = leRegistroSeq(fpEntrada);

        // detecta fim do arquivo após tentativa de leitura
        if (feof(fpEntrada)) {
            free(reg.nomeEstacao);
            free(reg.nomeLinha);
            break;
        }

        // ignora registros logicamente removidos
        if (reg.removido != '1') {
            // cresce o array dinamicamente se necessário
            if (total == capacidade) {
                capacidade *= 2;
                pares = realloc(pares, capacidade * sizeof(ParIndice));
            }
            pares[total].codEstacao = reg.codEstacao;
            pares[total].rrn = rrn;
            total++;
        }

        free(reg.nomeEstacao);
        free(reg.nomeLinha);
        rrn++;
    }

    // ordena os pares por codEstacao crescente
    qsort(pares, total, sizeof(ParIndice), comparaPar);

    // cria o arquivo de índice primário
    FILE *fpIndice = fopen(arquivoIndice, "wb");
    if (fpIndice == NULL) {
        printf("Falha no processamento do arquivo.\n");
        free(pares);
        fclose(fpEntrada);
        return 1;
    }

    // escreve o cabeçalho do índice: 1 byte de status = '1'
    char statusIndice = '1';
    fwrite(&statusIndice, sizeof(char), 1, fpIndice);

    // escreve cada par (codEstacao, RRN) como registro de 8 bytes
    for (int i = 0; i < total; i++) {
        fwrite(&pares[i].codEstacao, sizeof(int), 1, fpIndice);
        fwrite(&pares[i].rrn,        sizeof(int), 1, fpIndice);
    }

    // libera o array de pares e fecha os arquivos
    free(pares);
    fclose(fpEntrada);
    fclose(fpIndice);

    // exibe o conteúdo binário do arquivo de índice
    BinarioNaTela(arquivoIndice);
    return 0;
}