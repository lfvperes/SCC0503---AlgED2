/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registro.h"
#include "registro_io.h"
#include "filtro.h"

// converte o nome em string de um campo para o enum CampoRegistro
CampoRegistro nomeCampoParaEnum(char *nome) {
    if (strcmp(nome, "removido") == 0)        return CAMPO_REMOVIDO;
    if (strcmp(nome, "proximo") == 0)         return CAMPO_PROXIMO;
    if (strcmp(nome, "codEstacao") == 0)      return CAMPO_COD_ESTACAO;
    if (strcmp(nome, "codLinha") == 0)        return CAMPO_COD_LINHA;
    if (strcmp(nome, "codProxEstacao") == 0)  return CAMPO_COD_PROX_ESTACAO;
    if (strcmp(nome, "distProxEstacao") == 0) return CAMPO_DIST_PROX_ESTACAO;
    if (strcmp(nome, "codLinhaIntegra") == 0) return CAMPO_COD_LINHA_INTEGRA;
    if (strcmp(nome, "codEstIntegra") == 0)   return CAMPO_COD_EST_INTEGRA;
    if (strcmp(nome, "nomeEstacao") == 0)     return CAMPO_NOME_ESTACAO;
    if (strcmp(nome, "nomeLinha") == 0)       return CAMPO_NOME_LINHA;
    return -1; // campo desconhecido
}

// lê o campo indicado do registro na posição offsetRegistro e compara com valor.
// retorna 1 se o valor bate, 0 caso contrário.
int verificaCampo(FILE *fpBin, int offsetRegistro, CampoRegistro campo, char *valor) {
    int valorLido;
    int campoOffset;

    // NULO no input vira "" pelo ScanQuoteString — corresponde ao sentinela -1
    int valorEhNulo = (valor[0] == '\0') || (strcmp(valor, "NULO") == 0);

    switch (campo) {
        // campos inteiros fixos: apenas define o offset e cai na lógica comum abaixo
        case CAMPO_PROXIMO:
            campoOffset = OFFSET_PROXIMO;
            break;
        case CAMPO_COD_ESTACAO:
            campoOffset = OFFSET_COD_ESTACAO;
            break;
        case CAMPO_COD_LINHA:
            campoOffset = OFFSET_COD_LINHA;
            break;
        case CAMPO_COD_PROX_ESTACAO:
            campoOffset = OFFSET_COD_PROX_ESTACAO;
            break;
        case CAMPO_DIST_PROX_ESTACAO:
            campoOffset = OFFSET_DIST_PROX_ESTACAO;
            break;
        case CAMPO_COD_LINHA_INTEGRA:
            campoOffset = OFFSET_COD_LINHA_INTEGRA;
            break;
        case CAMPO_COD_EST_INTEGRA:
            campoOffset = OFFSET_COD_EST_INTEGRA;
            break;

        case CAMPO_REMOVIDO: {
            char removidoLido;
            fseek(fpBin, offsetRegistro + OFFSET_REMOVIDO, SEEK_SET);
            fread(&removidoLido, sizeof(char), 1, fpBin);
            return removidoLido == valor[0];
        }

        case CAMPO_NOME_ESTACAO: {
            int tamNomeEstacao;
            fseek(fpBin, offsetRegistro + OFFSET_TAM_NOME_ESTACAO, SEEK_SET);
            fread(&tamNomeEstacao, sizeof(int), 1, fpBin);

            char *nomeEstacao = malloc(tamNomeEstacao + 1);
            fseek(fpBin, offsetRegistro + OFFSET_NOME_ESTACAO, SEEK_SET);
            fread(nomeEstacao, tamNomeEstacao, 1, fpBin);
            nomeEstacao[tamNomeEstacao] = '\0';

            int resultado = strcmp(nomeEstacao, valor) == 0;
            free(nomeEstacao);
            return resultado;
        }

        case CAMPO_NOME_LINHA: {
            // precisa ler tamNomeEstacao para calcular onde nomeLinha começa
            int tamNomeEstacao;
            fseek(fpBin, offsetRegistro + OFFSET_TAM_NOME_ESTACAO, SEEK_SET);
            fread(&tamNomeEstacao, sizeof(int), 1, fpBin);

            int offsetTamNomeLinha = OFFSET_NOME_ESTACAO + tamNomeEstacao;

            int tamNomeLinha;
            fseek(fpBin, offsetRegistro + offsetTamNomeLinha, SEEK_SET);
            fread(&tamNomeLinha, sizeof(int), 1, fpBin);

            char *nomeLinha = malloc(tamNomeLinha + 1);
            fread(nomeLinha, tamNomeLinha, 1, fpBin);
            nomeLinha[tamNomeLinha] = '\0';

            int resultado = strcmp(nomeLinha, valor) == 0;
            free(nomeLinha);
            return resultado;
        }

        // CAMPO_TAM_NOME_ESTACAO e CAMPO_TAM_NOME_LINHA não são filtráveis
        default: return 0;
    }

    // lógica comum aos campos inteiros fixos
    fseek(fpBin, offsetRegistro + campoOffset, SEEK_SET);
    fread(&valorLido, sizeof(int), 1, fpBin);
    int valorComparar = valorEhNulo ? -1 : (int)strtol(valor, NULL, 10);
    return valorLido == valorComparar;
}

// percorre os registros do arquivo duas vezes: primeiro conta os que satisfazem
// todos os m filtros, depois aloca e preenche o array de resultados.
// retorna NULL se algum campo for desconhecido (encontrados != 0) ou
// se nenhum registro for encontrado (encontrados == 0).
struct registro* buscaRegistros(FILE *fpBin, char **nomeCampo, char **valorCampo,
                                int m, int nroRegistros, int *encontrados) {
    *encontrados = 0;

    // passe 1: conta os registros que passam no filtro
    for (int i = 0; i < nroRegistros; i++) {
        int offset = TAM_REG_CABECALHO + i * TAM_REG_DADOS;

        // ignora registros removidos
        char removido;
        fseek(fpBin, offset + OFFSET_REMOVIDO, SEEK_SET);
        fread(&removido, sizeof(char), 1, fpBin);
        if (removido == '1') continue;

        int satisfazTodos = 1;

        for (int j = 0; j < m; j++) {
            CampoRegistro campo = nomeCampoParaEnum(nomeCampo[j]);
            if ((int)campo == -1) {
                fprintf(stderr, "Campo desconhecido: %s\n", nomeCampo[j]);
                return NULL;
            }
            if (!verificaCampo(fpBin, offset, campo, valorCampo[j])) {
                satisfazTodos = 0;
                break; // AND: basta um falhar para descartar o registro
            }
        }

        if (satisfazTodos)
            (*encontrados)++;
    }

    if (*encontrados == 0)
        return NULL;

    // passe 2: aloca e preenche o array com os registros encontrados
    struct registro *resultado = malloc(*encontrados * sizeof(struct registro));
    int idx = 0;

    for (int i = 0; i < nroRegistros; i++) {
        int offset = TAM_REG_CABECALHO + i * TAM_REG_DADOS;

        char removido;
        fseek(fpBin, offset + OFFSET_REMOVIDO, SEEK_SET);
        fread(&removido, sizeof(char), 1, fpBin);
        if (removido == '1') continue;

        int satisfazTodos = 1;

        for (int j = 0; j < m; j++) {
            CampoRegistro campo = nomeCampoParaEnum(nomeCampo[j]);
            if (!verificaCampo(fpBin, offset, campo, valorCampo[j])) {
                satisfazTodos = 0;
                break;
            }
        }

        if (satisfazTodos)
            resultado[idx++] = leRegistro(fpBin, offset);
    }

    return resultado;
}
