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
#include "filtro.h"

// busca binária por codEstacao no array de pares (ordenado por codEstacao).
// retorna o RRN correspondente, ou -1 se não encontrado.
static int buscaBinariaIndice(ParIndice *pares, int total, int chave) {
    int inicio = 0, fim = total - 1;

    while (inicio <= fim) {
        int meio = inicio + (fim - inicio) / 2;

        if (pares[meio].codEstacao == chave)
            return pares[meio].rrn;
        else if (pares[meio].codEstacao < chave)
            inicio = meio + 1;
        else
            fim = meio - 1;
    }

    return -1;
}

int buscaIndice(char *arquivoEntrada, char *arquivoIndice, int n) {
    // abre o arquivo de índice
    FILE *fpIndice = fopen(arquivoIndice, "rb");
    if (fpIndice == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return 1;
    }

    // lê o status do índice
    char statusIndice;
    if (fread(&statusIndice, sizeof(char), 1, fpIndice) != 1 || statusIndice != '1') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fpIndice);
        return 1;
    }

    // calcula a quantidade de pares (codEstacao, RRN) a partir do tamanho do arquivo
    fseek(fpIndice, 0, SEEK_END);
    long tamArquivo = ftell(fpIndice);
    int total = (int)((tamArquivo - sizeof(char)) / (2 * sizeof(int)));

    // carrega os pares do índice em memória
    ParIndice *pares = malloc(total * sizeof(ParIndice));
    if (pares == NULL) {
        printf("Falha no processamento do arquivo.\n");
        fclose(fpIndice);
        return 1;
    }

    fseek(fpIndice, sizeof(char), SEEK_SET);
    for (int i = 0; i < total; i++) {
        fread(&pares[i].codEstacao, sizeof(int), 1, fpIndice);
        fread(&pares[i].rrn,        sizeof(int), 1, fpIndice);
    }
    fclose(fpIndice);

    // abre e valida o arquivo de dados
    FILE *fpEntrada = fopen(arquivoEntrada, "rb");
    if (fpEntrada == NULL) {
        printf("Falha no processamento do arquivo.\n");
        free(pares);
        return 1;
    }

    char status;
    int topo, proxRRN, nroEstacoes, nroParesEstacao;
    if (leCabecalho(fpEntrada, &status, &topo, &proxRRN, &nroEstacoes, &nroParesEstacao) != 0
        || status != '1') {
        printf("Falha no processamento do arquivo.\n");
        free(pares);
        fclose(fpEntrada);
        return 1;
    }

    // executa n buscas
    char bufferLinha[1024];
    for (int i = 0; i < n; i++) {
        int m;
        scanf("%d", &m);

        char **nomeCampo  = malloc(m * sizeof(char *));
        char **valorCampo = malloc(m * sizeof(char *));

        // lê os m pares nomeCampo/valorCampo
        int usaIndice = 0;
        int chave = -1;
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

            // verifica se algum campo de busca é codEstacao
            if (strcmp(nomeCampo[j], "codEstacao") == 0) {
                usaIndice = 1;
                chave = atoi(valorCampo[j]);
            }
        }

        int encontrados = 0;
        if (usaIndice) {
            // busca indexada: obtém RRN pelo índice e acessa diretamente
            int rrn = buscaBinariaIndice(pares, total, chave);
            if (rrn == -1) {
                encontrados = 0;
            } else {
                int offset = TAM_REG_CABECALHO + rrn * TAM_REG_DADOS;
                struct registro dados = leRegistro(fpEntrada, offset);
                if (dados.removido != '1') {
                    imprimeRegistro(dados);
                    encontrados = 1;
                }
                free(dados.nomeEstacao);
                free(dados.nomeLinha);
            }
        } else {
            // busca sequencial para os demais campos
            encontrados = buscaRegistros(fpEntrada, nomeCampo, valorCampo, m, proxRRN);
        }

        if (encontrados == 0)
            printf("Registro inexistente.\n");
        else if (encontrados == -1)
            printf("Falha no processamento do arquivo.\n");

        for (int j = 0; j < m; j++) {
            free(nomeCampo[j]);
            free(valorCampo[j]);
        }
        free(nomeCampo);
        free(valorCampo);

        if (i < n - 1)
            printf("\n");
    }

    free(pares);
    fclose(fpEntrada);
    return 0;
}