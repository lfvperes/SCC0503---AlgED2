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
int buscaBinariaIndice(ParIndice *pares, int total, int chave) {
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

// carrega os pares do arquivo de índice em memória.
// retorna o array alocado e preenche *total com a quantidade de pares.
// retorna NULL em caso de erro.
ParIndice *carregaIndice(char *arquivoIndice, int *total) {
    FILE *fpIndice = fopen(arquivoIndice, "rb");
    if (fpIndice == NULL)
        return NULL;

    // valida o status do índice
    char statusIndice;
    if (fread(&statusIndice, sizeof(char), 1, fpIndice) != 1 || statusIndice != '1') {
        fclose(fpIndice);
        return NULL;
    }

    // calcula a quantidade de pares a partir do tamanho do arquivo
    fseek(fpIndice, 0, SEEK_END);
    long tamArquivo = ftell(fpIndice);
    *total = (int)((tamArquivo - sizeof(char)) / (2 * sizeof(int)));

    // aloca e carrega os pares em memória
    ParIndice *pares = malloc(*total * sizeof(ParIndice));
    if (pares == NULL) {
        fclose(fpIndice);
        return NULL;
    }

    fseek(fpIndice, sizeof(char), SEEK_SET);
    for (int i = 0; i < *total; i++) {
        fread(&pares[i].codEstacao, sizeof(int), 1, fpIndice);
        fread(&pares[i].rrn,        sizeof(int), 1, fpIndice);
    }

    fclose(fpIndice);
    return pares;
}

int buscaIndice(char *arquivoEntrada, char *arquivoIndice, int n) {
    // carrega o índice inteiramente em memória RAM
    int total;
    ParIndice *pares = carregaIndice(arquivoIndice, &total);
    if (pares == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return 1;
    }

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
            // busca indexada: usa o índice para localizar o RRN diretamente,
            // depois verifica TODOS os m filtros no registro carregado
            int rrn = buscaBinariaIndice(pares, total, chave);
            if (rrn == -1) {
                encontrados = 0;
            } else {
                int offset = TAM_REG_CABECALHO + rrn * TAM_REG_DADOS;
                struct registro dados = leRegistro(fpEntrada, offset);
                if (dados.removido != '1') {
                    int satisfazTodos = 1;
                    for (int j = 0; j < m; j++) {
                        CampoRegistro campo = nomeCampoParaEnum(nomeCampo[j]);
                        if (!verificaCampoMem(&dados, campo, valorCampo[j])) {
                            satisfazTodos = 0;
                            break;
                        }
                    }
                    if (satisfazTodos) {
                        imprimeRegistro(dados);
                        encontrados = 1;
                    }
                }
                free(dados.nomeEstacao);
                free(dados.nomeLinha);
            }
        } else {
            // busca sequencial para os demais campos
            encontrados = buscaRegistros(fpEntrada, nomeCampo, valorCampo, m, proxRRN, MODO_IMPRIMIR, NULL);
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