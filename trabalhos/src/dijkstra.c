/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "dijkstra.h"
#include "grafo.h"
#include "geraGrafo.h"
#include "heap.h"

// executa o algoritmo de Dijkstra a partir de idxOrigem
// usa inserção duplicada no heap (sem decrease-key): a cada relaxamento
// bem-sucedido, insere um novo nó; ao extrair, ignora vértices já finalizados
void dijkstra(Grafo *g, int idxOrigem, int *dist, int *pred) {
    int *finalizado = calloc(g->nVertices, sizeof(int));

    // inicializa distâncias como "infinito" e predecessores como inexistentes
    for (int i = 0; i < g->nVertices; i++) {
        dist[i] = INT_MAX;
        pred[i] = -1;
    }
    dist[idxOrigem] = 0;

    MinHeap *h = criaHeap();
    insereHeap(h, idxOrigem, 0, g->vertices[idxOrigem].nomeEstacao);

    while (!heapVazio(h)) {
        HeapNode no = extraiMin(h);

        // vértice já finalizado com distância menor ou igual — ignora (lazy deletion)
        if (finalizado[no.idx]) continue;
        finalizado[no.idx] = 1;

        // relaxa as arestas na ordem em que já estão na lista (ordenada por
        // nomeProxEstacao), garantindo o desempate de menor v em caso de
        // arestas de mesmo peso
        ArestaAdj *a = g->vertices[no.idx].lista;
        while (a != NULL) {
            int idxV = buscaVertice(g, a->nomeProxEstacao);

            if (idxV != -1 && !finalizado[idxV]) {
                int novaDist = dist[no.idx] + a->distancia;
                if (novaDist < dist[idxV]) {
                    dist[idxV] = novaDist;
                    pred[idxV] = no.idx;
                    insereHeap(h, idxV, novaDist, g->vertices[idxV].nomeEstacao);
                }
            }

            a = a->prox;
        }
    }

    liberaHeap(h);
    free(finalizado);
}

// reconstrói o caminho de idxOrigem até idxDestino a partir do vetor pred,
// retornando um vetor de índices na ordem origem -> destino e escrevendo o
// tamanho do caminho (número de vértices, incluindo origem e destino) em *tam
static int *reconstroiCaminho(int *pred, int idxOrigem, int idxDestino, int *tam) {
    // conta quantos vértices há no caminho, andando de destino até origem
    int n = 0;
    int atual = idxDestino;
    while (atual != -1) {
        n++;
        if (atual == idxOrigem) break;
        atual = pred[atual];
    }

    int *caminho = malloc(n * sizeof(int));
    atual = idxDestino;
    for (int i = n - 1; i >= 0; i--) {
        caminho[i] = atual;
        atual = pred[atual];
    }

    *tam = n;
    return caminho;
}

// funcionalidade 11: constrói o grafo, executa Dijkstra e imprime o resultado
void funcionalidade11(char *arquivoBin, char *nomeOrigem, char *nomeDestino) {
    Grafo *g = constroiGrafo(arquivoBin);
    if (g == NULL) {
        printf("Falha na execucao da funcionalidade.\n");
        return;
    }

    int idxOrigem = buscaVertice(g, nomeOrigem);
    int idxDestino = buscaVertice(g, nomeDestino);
    if (idxOrigem == -1 || idxDestino == -1) {
        printf("Falha na execucao da funcionalidade.\n");
        liberaGrafo(g);
        return;
    }

    int *dist = malloc(g->nVertices * sizeof(int));
    int *pred = malloc(g->nVertices * sizeof(int));
    dijkstra(g, idxOrigem, dist, pred);

    if (dist[idxDestino] == INT_MAX) {
        printf("Nao existe caminho entre as estacoes solicitadas.\n");
    } else {
        int tam;
        int *caminho = reconstroiCaminho(pred, idxOrigem, idxDestino, &tam);

        // numero de estacoes percorridas nao conta a origem, mas conta o destino
        printf("Numero de estacoes que serao percorridas: %d\n", tam - 1);
        printf("Distancia que sera percorrida: %d\n", dist[idxDestino]);

        for (int i = 0; i < tam; i++) {
            printf("%s", g->vertices[caminho[i]].nomeEstacao);
            if (i < tam - 1) printf(", ");
        }
        printf("\n");

        free(caminho);
    }

    free(dist);
    free(pred);
    liberaGrafo(g);
}
