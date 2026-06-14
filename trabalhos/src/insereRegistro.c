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

int insertRegistro(char *arquivoEntrada, char *arquivoIndice) {
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

    ScanQuoteString(bufStr);
    novo.codProxEstacao = (strlen(bufStr) == 0) ? -1 : atoi(bufStr);

    ScanQuoteString(bufStr);
    novo.distProxEstacao = (strlen(bufStr) == 0) ? -1 : atoi(bufStr);

    ScanQuoteString(bufStr);
    novo.codLinhaIntegra = (strlen(bufStr) == 0) ? -1 : atoi(bufStr);

    ScanQuoteString(bufStr);
    novo.codEstIntegra = (strlen(bufStr) == 0) ? -1 : atoi(bufStr);

    novo.removido = '0';
    novo.proximo  = -1;

    // --- verifica duplicata via busca binária no índice ---
    if (buscaBinariaIndice(pares, total, novo.codEstacao) != -1) {
        printf("Chave já existente: registro com codEstacao %d não inserido.\n",
               novo.codEstacao);
        free(novo.nomeEstacao);
        free(novo.nomeLinha);
        free(pares);
        fclose(fpDados);
        return 1;
    }

    // --- marca o arquivo de dados como inconsistente durante a escrita ---
    escreveCabecalho(fpDados, '0', -1, proxRRN, nroEstacoes, nroParesEstacao);

    // posiciona no fim do arquivo de dados e escreve o novo registro
    int novoRRN = proxRRN;
    fseek(fpDados, 0, SEEK_END);
    escreveRegistroDados(fpDados, &novo);

    // atualiza o cabeçalho: proxRRN incrementado, arquivo marcado como consistente
    escreveCabecalho(fpDados, '1', -1, proxRRN + 1, nroEstacoes, nroParesEstacao);
    fclose(fpDados);

    // --- insere o novo par no índice mantendo a ordem crescente por codEstacao ---
    // realoca o array com uma posição extra
    pares = realloc(pares, (total + 1) * sizeof(ParIndice));

    // encontra a posição de inserção (busca linear — o array já está ordenado)
    int posInsert = total;
    for (int i = 0; i < total; i++) {
        if (pares[i].codEstacao > novo.codEstacao) {
            posInsert = i;
            break;
        }
    }

    // desloca os elementos maiores para abrir espaço
    for (int i = total; i > posInsert; i--)
        pares[i] = pares[i - 1];

    // insere o novo par na posição correta
    pares[posInsert].codEstacao = novo.codEstacao;
    pares[posInsert].rrn        = novoRRN;

    // grava o índice atualizado em disco
    if (salvaPares(arquivoIndice, pares, total + 1) != 0) {
        printf("Falha no processamento do arquivo.\n");
        free(novo.nomeEstacao);
        free(novo.nomeLinha);
        free(pares);
        return 1;
    }

    free(novo.nomeEstacao);
    free(novo.nomeLinha);
    free(pares);

    // exibe checksums dos dois arquivos atualizados
    BinarioNaTela(arquivoEntrada);
    BinarioNaTela(arquivoIndice);
    return 0;
}