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

// verifica se um registro com o mesmo nomeEstacao já foi escrito no arquivo.
// retorna 1 se encontrado, 0 caso contrário.
static int nomeEstacaoExiste(FILE *fpBin, const struct registro *dados, int nroRegistros) {
    int bufferTamNomeEstacao, n;

    for (int i = 0; i < nroRegistros; i++) {
        int offset = TAM_REG_CABECALHO + i * TAM_REG_DADOS + OFFSET_TAM_NOME_ESTACAO;
        fseek(fpBin, offset, SEEK_SET);
        fread(&bufferTamNomeEstacao, sizeof(int), 1, fpBin);

        if (bufferTamNomeEstacao == dados->tamNomeEstacao) {
            char bufferNomeEstacao[bufferTamNomeEstacao];
            fread(bufferNomeEstacao, bufferTamNomeEstacao, 1, fpBin);
            n = memcmp(bufferNomeEstacao, dados->nomeEstacao, dados->tamNomeEstacao);
            if (n == 0) {
                fseek(fpBin, TAM_REG_CABECALHO + nroRegistros * TAM_REG_DADOS, SEEK_SET);
                return 1;
            }
        }
    }

    fseek(fpBin, TAM_REG_CABECALHO + nroRegistros * TAM_REG_DADOS, SEEK_SET);
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

// verifica se um par de estações já existe no arquivo.
// retorna 1 se encontrado, 0 caso contrário.
static int parExiste(FILE *fpBin, struct parEstacao parNovo, int nroRegistros) {
    int codEstacaoDisco, codProxEstacaoDisco;
    int offsetCodEstacao    = sizeof(char) + sizeof(int);
    int offsetCodProxEstacao = offsetCodEstacao + sizeof(int) + sizeof(int);
    struct parEstacao parDisco;

    for (int i = 0; i < nroRegistros; i++) {
        int base = TAM_REG_CABECALHO + i * TAM_REG_DADOS;

        fseek(fpBin, base + offsetCodEstacao, SEEK_SET);
        fread(&codEstacaoDisco, sizeof(int), 1, fpBin);

        fseek(fpBin, base + offsetCodProxEstacao, SEEK_SET);
        fread(&codProxEstacaoDisco, sizeof(int), 1, fpBin);

        parDisco.codEstacao1 = codEstacaoDisco;
        parDisco.codEstacao2 = codProxEstacaoDisco;

        if (paresSaoEquivalentes(parDisco, parNovo)) {
            fseek(fpBin, TAM_REG_CABECALHO + nroRegistros * TAM_REG_DADOS, SEEK_SET);
            return 1;
        }
    }

    fseek(fpBin, TAM_REG_CABECALHO + nroRegistros * TAM_REG_DADOS, SEEK_SET);
    return 0;
}

// lê registros do arquivo CSV e escreve no arquivo binário.
// o cabeçalho é atualizado após cada registro escrito para manter
// consistência em caso de falha. ao final marca o arquivo como consistente.
int criaTabela(char *estacoesCSV, char *estacoesBin) {
    struct registro dados;
    struct parEstacao parNovo;
    int nroRegistros = 0, nroEstacoes = 0, nroParesEstacao = 0;

    FILE *fpCSV = fopen(estacoesCSV, "r");
    FILE *fpBin = fopen(estacoesBin, "wb+");

    if (fpCSV == NULL) {
        printf("Falha no processamento do arquivo.");
        return 1;
    }
    if (fpBin == NULL) {
        perror("Erro ao abrir arquivo binario para escrita");
        return 1;
    }

    // escreve cabeçalho inicial com status '0' (inconsistente) e contadores zerados
    escreveCabecalho(fpBin, '0', 0, 0, 0);

    char linha[TAM_REG_DADOS];
    descartaCabecalhoCSV(fpCSV);

    while (fgets(linha, TAM_REG_DADOS, fpCSV)) {
        linha[strcspn(linha, "\r\n")] = '\0';
        lerRegistroCSV(linha, &dados);

        if (!nomeEstacaoExiste(fpBin, &dados, nroRegistros))
            nroEstacoes++;

        parNovo.codEstacao1 = dados.codEstacao;
        parNovo.codEstacao2 = dados.codProxEstacao;
        if (dados.codProxEstacao != -1 && !parExiste(fpBin, parNovo, nroRegistros))
            nroParesEstacao++;

        escreveRegistroDados(fpBin, &dados);
        nroRegistros++;

        // atualiza o cabeçalho após cada escrita
        escreveCabecalho(fpBin, '0', nroRegistros, nroEstacoes, nroParesEstacao);

        free(dados.nomeEstacao);
        free(dados.nomeLinha);
    }

    // marca o arquivo como consistente ao final da escrita
    escreveCabecalho(fpBin, '1', nroRegistros, nroEstacoes, nroParesEstacao);

    fclose(fpCSV);
    fclose(fpBin);

    BinarioNaTela(estacoesBin);
    return 0;
}
