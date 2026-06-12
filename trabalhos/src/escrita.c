/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registro.h"
#include "arquivo_bin.h"
#include "csv_parser.h"
#include "fornecidas.h"
#include "escrita.h"

// serializa um registro de dados em um buffer de tamanho fixo (TAM_REG_DADOS)
// e escreve no arquivo binário. bytes restantes são preenchidos com $
static void escreveRegistroDados(FILE *fpBin, const struct registro *dados) {
    char bufferDados[TAM_REG_DADOS];
    memset(bufferDados, 0, TAM_REG_DADOS);

    size_t offset = 0;

    memcpy(bufferDados + offset, &dados->removido, sizeof(dados->removido));
    offset += sizeof(dados->removido);

    memcpy(bufferDados + offset, &dados->proximo, sizeof(dados->proximo));
    offset += sizeof(dados->proximo);

    memcpy(bufferDados + offset, &dados->codEstacao, sizeof(dados->codEstacao));
    offset += sizeof(dados->codEstacao);

    memcpy(bufferDados + offset, &dados->codLinha, sizeof(dados->codLinha));
    offset += sizeof(dados->codLinha);

    memcpy(bufferDados + offset, &dados->codProxEstacao, sizeof(dados->codProxEstacao));
    offset += sizeof(dados->codProxEstacao);

    memcpy(bufferDados + offset, &dados->distProxEstacao, sizeof(dados->distProxEstacao));
    offset += sizeof(dados->distProxEstacao);

    memcpy(bufferDados + offset, &dados->codLinhaIntegra, sizeof(dados->codLinhaIntegra));
    offset += sizeof(dados->codLinhaIntegra);

    memcpy(bufferDados + offset, &dados->codEstIntegra, sizeof(dados->codEstIntegra));
    offset += sizeof(dados->codEstIntegra);

    // campos de tamanho variável — precedidos pelo tamanho em bytes
    memcpy(bufferDados + offset, &dados->tamNomeEstacao, sizeof(dados->tamNomeEstacao));
    offset += sizeof(dados->tamNomeEstacao);

    memcpy(bufferDados + offset, dados->nomeEstacao, dados->tamNomeEstacao);
    offset += dados->tamNomeEstacao;

    memcpy(bufferDados + offset, &dados->tamNomeLinha, sizeof(dados->tamNomeLinha));
    offset += sizeof(dados->tamNomeLinha);

    if (dados->tamNomeLinha > 0)
        memcpy(bufferDados + offset, dados->nomeLinha, dados->tamNomeLinha);
    offset += dados->tamNomeLinha;

    // bytes restantes preenchidos com $
    memset(bufferDados + offset, '$', TAM_REG_DADOS - offset);

    if (fwrite(bufferDados, TAM_REG_DADOS, 1, fpBin) != 1)
        perror("Erro ao escrever registro de dados no arquivo binario");
}

static int nomeEstacaoExiste(char **nomesVistos, int nroNomes, const struct registro *dados) {
    for (int i = 0; i < nroNomes; i++) {
        // compara apenas tamNomeEstacao bytes pois nomeEstacao não tem null terminator
        if (strncmp(nomesVistos[i], dados->nomeEstacao, dados->tamNomeEstacao) == 0
                && (int)strlen(nomesVistos[i]) == dados->tamNomeEstacao)
            return 1;
    }
    return 0;
}

// ordena par para normalização
static void normalizaPar(struct parEstacao *par) {
    if (par->codEstacao1 > par->codEstacao2) {
        int aux = par->codEstacao1;
        par->codEstacao1 = par->codEstacao2;
        par->codEstacao2 = aux;
    }
}

// retorna 1 se os pares são equivalentes após normalizar
static int paresSaoEquivalentes(struct parEstacao parDisco, struct parEstacao parNovo) {
    normalizaPar(&parDisco);
    normalizaPar(&parNovo);
    return (parDisco.codEstacao1 == parNovo.codEstacao1 &&
            parDisco.codEstacao2 == parNovo.codEstacao2);
}

// verifica se par já existe no array em memória.
// retorna 1 se encontrado, 0 caso contrário.
static int parExiste(struct parEstacao *paresVistos, int nroPares, struct parEstacao parNovo) {
    for (int i = 0; i < nroPares; i++) {
        if (paresSaoEquivalentes(paresVistos[i], parNovo))
            return 1;
    }
    return 0;
}

// lê registros do arquivo CSV e escreve no arquivo binário.
// o cabeçalho é atualizado após cada registro escrito para manter
// consistência em caso de falha. ao final marca o arquivo como consistente.
int criaTabela(char *estacoesCSV, char *estacoesBin) {
    struct registro dados;
    struct parEstacao parNovo;
    int nroRegistros = 0, nroEstacoes = 0, nroParesEstacao = 0;
    char **nomesVistos = NULL;
    struct parEstacao *paresVistos = NULL;

    FILE *fpCSV = fopen(estacoesCSV, "r");
    FILE *fpBin = fopen(estacoesBin, "wb");

    if (fpCSV == NULL) {
        printf("Falha no processamento do arquivo.");
        return 1;
    }
    if (fpBin == NULL) {
        perror("Erro ao abrir arquivo binario para escrita");
        return 1;
    }

    // escreve cabeçalho inicial com status '0' (inconsistente) e contadores zerados
    escreveCabecalho(fpBin, '0', -1, 0, 0, 0);

    char linha[TAM_REG_DADOS];
    descartaCabecalhoCSV(fpCSV);

    while (fgets(linha, TAM_REG_DADOS, fpCSV)) {
        linha[strcspn(linha, "\r\n")] = '\0';
        lerRegistroCSV(linha, &dados);

        // se o nome da estação ainda não foi visto, adiciona ao array e incrementa o contador
        if (!nomeEstacaoExiste(nomesVistos, nroEstacoes, &dados)) {
            nomesVistos = realloc(nomesVistos, (nroEstacoes + 1) * sizeof(char *));
            nomesVistos[nroEstacoes] = malloc(dados.tamNomeEstacao + 1);
            memcpy(nomesVistos[nroEstacoes], dados.nomeEstacao, dados.tamNomeEstacao);
            nomesVistos[nroEstacoes][dados.tamNomeEstacao] = '\0';
            nroEstacoes++;
        }

        parNovo.codEstacao1 = dados.codEstacao;
        parNovo.codEstacao2 = dados.codProxEstacao;
        if (dados.codProxEstacao != -1 && !parExiste(paresVistos, nroParesEstacao, parNovo)) {
            paresVistos = realloc(paresVistos, (nroParesEstacao + 1) * sizeof(struct parEstacao));
            paresVistos[nroParesEstacao] = parNovo;
            nroParesEstacao++;
        }

        escreveRegistroDados(fpBin, &dados);
        nroRegistros++;


        free(dados.nomeEstacao);
        free(dados.nomeLinha);
    }

    // marca o arquivo como consistente ao final da escrita
    escreveCabecalho(fpBin, '1', -1, nroRegistros, nroEstacoes, nroParesEstacao);

    for (int i = 0; i < nroEstacoes; i++)
        free(nomesVistos[i]);
    free(nomesVistos);
    free(paresVistos);

    fclose(fpCSV);
    fclose(fpBin);

    BinarioNaTela(estacoesBin);
    return 0;
}
