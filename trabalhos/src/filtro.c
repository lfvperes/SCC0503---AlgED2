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
    return -1;
}

// compara um campo do registro (já em memória) com o valor fornecido como string
static int verificaCampoMem(const struct registro *reg, CampoRegistro campo, char *valor) {
    int valorEhNulo = (valor[0] == '\0') || (strcmp(valor, "NULO") == 0);
    int valorInt = valorEhNulo ? -1 : (int)strtol(valor, NULL, 10);

    switch (campo) {
        case CAMPO_REMOVIDO:          return reg->removido == valor[0];
        case CAMPO_PROXIMO:           return reg->proximo == valorInt;
        case CAMPO_COD_ESTACAO:       return reg->codEstacao == valorInt;
        case CAMPO_COD_LINHA:         return reg->codLinha == valorInt;
        case CAMPO_COD_PROX_ESTACAO:  return reg->codProxEstacao == valorInt;
        case CAMPO_DIST_PROX_ESTACAO: return reg->distProxEstacao == valorInt;
        case CAMPO_COD_LINHA_INTEGRA: return reg->codLinhaIntegra == valorInt;
        case CAMPO_COD_EST_INTEGRA:   return reg->codEstIntegra == valorInt;
        case CAMPO_NOME_ESTACAO:      return strcmp(reg->nomeEstacao, valor) == 0;
        case CAMPO_NOME_LINHA:        return strcmp(reg->nomeLinha, valor) == 0;
        default:                      return 0;
    }
}

// percorre os registros sequencialmente, imprimindo cada um que satisfaz o filtro.
// usa fseek apenas para pular registros removidos.
// retorna o número de registros encontrados, ou -1 em caso de erro.
int buscaRegistros(FILE *fpBin, char **nomeCampo, char **valorCampo, int m, int nroRegistros, ModoBusca modo, int **rrns) {
    int encontrados = 0;

    // valida os campos antes de varrer o arquivo
    CampoRegistro campos[m];
    for (int j = 0; j < m; j++) {
        campos[j] = nomeCampoParaEnum(nomeCampo[j]);
        if ((int)campos[j] == -1) {
            fprintf(stderr, "Campo desconhecido: %s\n", nomeCampo[j]);
            return -1;
        }
    }

    // verifica se codEstacao está entre os filtros (é único, então para ao encontrar)
    int filtraPorId = 0;
    for (int j = 0; j < m; j++) {
        if (campos[j] == CAMPO_COD_ESTACAO) {
            filtraPorId = 1;
            break;
        }
    }

    fseek(fpBin, TAM_REG_CABECALHO, SEEK_SET);
    for (int i = 0; i < nroRegistros; i++) {
        long posInicio = ftell(fpBin);
        struct registro reg = leRegistroSeq(fpBin);

        if (reg.removido == '1') {
            free(reg.nomeEstacao);
            free(reg.nomeLinha);
            fseek(fpBin, posInicio + TAM_REG_DADOS, SEEK_SET);
            continue;
        }

        int satisfazTodos = 1;
        for (int j = 0; j < m; j++) {
            if (!verificaCampoMem(&reg, campos[j], valorCampo[j])) {
                satisfazTodos = 0;
                break;
            }
        }

        if (satisfazTodos) {
            if (modo == MODO_IMPRIMIR) {
                imprimeRegistro(reg);
            } else {
                // cresce o array de RRNs dinamicamente
                *rrns = realloc(*rrns, (encontrados + 1) * sizeof(int));
                (*rrns)[encontrados] = i;
            }
            encontrados++;
        }

        // libera memória do registro atual antes de seguir
        free(reg.nomeEstacao);
        free(reg.nomeLinha);

        if (filtraPorId && satisfazTodos) break;
    }

    return encontrados;
}