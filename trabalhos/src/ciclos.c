/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ciclos.h"
#include "grafo.h"
#include "geraGrafo.h"

// DFS recursivo com backtracking para contar ciclos simples a partir da origem
void dfsCiclos(Grafo *g, int idxOrigem, int idxAtual, int *visitado, int *count) {
    ArestaAdj *a = g->vertices[idxAtual].lista;

    while (a != NULL) {
        int idxV = buscaVertice(g, a->nomeProxEstacao);

        if (idxV == idxOrigem) {
            // encontrou um ciclo, volta a origem
            (*count)++;
        } else if (idxV != -1 && !visitado[idxV]) {
            // vértice intermediário nao visitado, continua explorando
            visitado[idxV] = 1;
            dfsCiclos(g, idxOrigem, idxV, visitado, count);
            visitado[idxV] = 0; // backtrack
        }

        a = a->prox;
    }
}

// funcionalidade 13: constroi o grafo, localiza a origem e conta ciclos simples
void funcionalidade13(char *arquivoBin, char *nomeOrigem) {
    Grafo *g = constroiGrafo(arquivoBin);
    if (g == NULL) {
        printf("Falha na execucao da funcionalidade.\n");
        return;
    }

    int idxOrigem = buscaVertice(g, nomeOrigem);
    if (idxOrigem == -1) {
        printf("Falha na execucao da funcionalidade.\n");
        liberaGrafo(g);
        return;
    }

    // array de visitados inicializado com zeros
    int *visitado = calloc(g->nVertices, sizeof(int));

    int count = 0;
    dfsCiclos(g, idxOrigem, idxOrigem, visitado, &count);

    if (count == 0)
        printf("Quantidade de ciclos: -1\n");
    else
        printf("Quantidade de ciclos: %d\n", count);

    free(visitado);
    liberaGrafo(g);
}