/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grafo.h"
#include "geraGrafo.h"
#include "arquivo_bin.h"
#include "registro_io.h"

// par (código, nome) para resolver códigos de estação em nomes
typedef struct {
    int cod;
    char *nome;
} MapaCodNome;

// busca linear no mapa; retorna o nome associado ao código ou NULL se não encontrado
static char *buscaMapa(MapaCodNome *mapa, int total, int cod) {
    for (int i = 0; i < total; i++) {
        if (mapa[i].cod == cod) return mapa[i].nome;
    }
    return NULL;
}

// garante que um vértice com o nome dado existe no grafo; insere se necessário
static void garanteVertice(Grafo *g, char *nome) {
    if (buscaVertice(g, nome) == -1) {
        insereVertice(g, nome);
    }
}

// passagem 1: lê todos os registros ativos e monta o mapa cod → nome
// retorna o mapa alocado e escreve o total em *totalMapa
static MapaCodNome *constroiMapa(FILE *fp, int *totalMapa) {
    int capacidade = 16;
    int total = 0;
    MapaCodNome *mapa = malloc(capacidade * sizeof(MapaCodNome));

    fseek(fp, TAM_REG_CABECALHO, SEEK_SET);

    while (1) {
        struct registro reg = leRegistroSeq(fp);

        if (feof(fp)) {
            free(reg.nomeEstacao);
            free(reg.nomeLinha);
            break;
        }

        if (reg.removido == '1') {
            free(reg.nomeEstacao);
            free(reg.nomeLinha);
            continue;
        }

        if (total == capacidade) {
            capacidade *= 2;
            mapa = realloc(mapa, capacidade * sizeof(MapaCodNome));
        }

        mapa[total].cod  = reg.codEstacao;
        mapa[total].nome = reg.nomeEstacao; // transfere ownership — não liberar aqui
        total++;

        free(reg.nomeLinha);
    }

    *totalMapa = total;
    return mapa;
}

// passagem 2: constrói o grafo usando o mapa de códigos
static void preencheGrafo(FILE *fp, Grafo *g, MapaCodNome *mapa, int totalMapa) {
    fseek(fp, TAM_REG_CABECALHO, SEEK_SET);

    while (1) {
        struct registro reg = leRegistroSeq(fp);

        if (feof(fp)) {
            free(reg.nomeEstacao);
            free(reg.nomeLinha);
            break;
        }

        if (reg.removido == '1') {
            free(reg.nomeEstacao);
            free(reg.nomeLinha);
            continue;
        }

        char *nomeU = reg.nomeEstacao;
        garanteVertice(g, nomeU);

        // aresta para próxima estação da linha
        if (reg.codProxEstacao != -1) {
            char *nomeV = buscaMapa(mapa, totalMapa, reg.codProxEstacao);
            if (nomeV != NULL) {
                garanteVertice(g, nomeV);
                insereAresta(g, nomeU, nomeV, reg.distProxEstacao, reg.nomeLinha);
            }
        }

        // aresta de integração (apenas se nomes forem diferentes)
        if (reg.codEstIntegra != -1) {
            char *nomeV = buscaMapa(mapa, totalMapa, reg.codEstIntegra);
            if (nomeV != NULL && strcmp(nomeU, nomeV) != 0) {
                garanteVertice(g, nomeV);
                insereAresta(g, nomeU, nomeV, 0, "Integração");
            }
        }

        free(reg.nomeEstacao);
        free(reg.nomeLinha);
    }
}

// constrói e retorna o grafo completo a partir do arquivo binário
// retorna NULL em caso de erro
Grafo *constroiGrafo(char *arquivoBin) {
    FILE *fp = fopen(arquivoBin, "rb");
    if (fp == NULL) return NULL;

    char status;
    int topo, proxRRN, nroEstacoes, nroParesEstacao;
    if (leCabecalho(fp, &status, &topo, &proxRRN, &nroEstacoes, &nroParesEstacao) != 0
        || status != '1') {
        fclose(fp);
        return NULL;
    }

    int totalMapa = 0;
    MapaCodNome *mapa = constroiMapa(fp, &totalMapa);

    Grafo *g = criaGrafo();
    preencheGrafo(fp, g, mapa, totalMapa);

    // libera o mapa (os nomes foram copiados pelo grafo via strcpy em insereVertice)
    for (int i = 0; i < totalMapa; i++) {
        free(mapa[i].nome);
    }
    free(mapa);

    fclose(fp);
    return g;
}

// funcionalidade 10: constrói o grafo e imprime
void funcionalidade10(char *arquivoBin) {
    Grafo *g = constroiGrafo(arquivoBin);
    if (g == NULL) {
        printf("Falha na execucao da funcionalidade.\n");
        return;
    }
    imprimeGrafo(g);
    liberaGrafo(g);
}