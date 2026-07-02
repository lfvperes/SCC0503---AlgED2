/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "mst.h"
#include "grafo.h"
#include "geraGrafo.h"
#include "heap.h"

// Constrói uma matriz de adjacência não-direcionada (n x n) a partir do grafo
// direcionado g. Para cada aresta (u -> v, peso) presente na lista de
// adjacências de g, registra o mesmo peso tanto em matriz[u][v] quanto em
// matriz[v][u], já que a especificação da funcionalidade 12 trata as linhas
// do metrô como bidirecionais (é possível ir de uma estação a outra e
// vice-versa). Posições sem aresta recebem o valor sentinela -1.
static int **constroiMatrizNaoDirecionada(Grafo *g) {
    int n = g->nVertices;

    int **matriz = malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++) {
        matriz[i] = malloc(n * sizeof(int));
        for (int j = 0; j < n; j++) {
            matriz[i][j] = -1; // -1 indica ausência de aresta entre i e j
        }
    }

    for (int u = 0; u < n; u++) {
        ArestaAdj *a = g->vertices[u].lista;
        while (a != NULL) {
            int v = buscaVertice(g, a->nomeProxEstacao);
            if (v != -1) {
                matriz[u][v] = a->distancia;
                matriz[v][u] = a->distancia; // torna a aresta bidirecional
            }
            a = a->prox;
        }
    }

    return matriz;
}

// libera a matriz de adjacência não-direcionada
static void liberaMatriz(int **matriz, int n) {
    for (int i = 0; i < n; i++) {
        free(matriz[i]);
    }
    free(matriz);
}

// Executa o algoritmo de Prim a partir do vértice idxOrigem, usando o MinHeap
// (heap.h/heap.c) para escolher, a cada passo, o vértice de menor distância
// acumulada até a árvore — o próprio heap já desempata pelo nome da estação
// quando há distâncias iguais, conforme exigido pela especificação.
//
// Preenche pai[v] com o índice do vértice pai de v na MST (ou -1 para a
// origem e para vértices não alcançáveis a partir dela) e pesoAresta[v] com
// o peso da aresta (pai[v], v).
//
// Ponto de atenção: o campo nome de HeapNode é apenas uma referência para a
// string nomeEstacao já alocada dentro do Grafo — o heap não aloca nem libera
// essa memória. Por isso só inserimos no heap vértices que já existem em g
// (índices do próprio vetor g->vertices, construído antes de chamar prim()).
static void prim(Grafo *g, int **matriz, int idxOrigem, int *pai, int *pesoAresta) {
    int n = g->nVertices;

    int *dist = malloc(n * sizeof(int));
    int *naMST = calloc(n, sizeof(int));

    for (int i = 0; i < n; i++) {
        dist[i] = INT_MAX;
        pai[i] = -1;
    }
    dist[idxOrigem] = 0;

    MinHeap *heap = criaHeap();
    // nome referencia a string já alocada em g->vertices[idxOrigem] (vértice já existe)
    insereHeap(heap, idxOrigem, 0, g->vertices[idxOrigem].nomeEstacao);

    while (!heapVazio(heap)) {
        HeapNode min = extraiMin(heap);
        int u = min.idx;

        // exclusão preguiçosa: este vértice já foi definitivamente incorporado
        // à MST por uma extração anterior; esta entrada é obsoleta
        if (naMST[u]) continue;
        naMST[u] = 1;

        // relaxa todos os vizinhos (não-direcionados) de u ainda fora da MST
        for (int v = 0; v < n; v++) {
            if (matriz[u][v] == -1 || naMST[v]) continue;

            int peso = matriz[u][v];

            // adota a aresta (u,v) como melhor forma de alcançar v se:
            // (a) o peso for estritamente menor que a melhor distância atual; ou
            // (b) houver empate no peso e u tiver nome menor que o pai atual de
            //     v (desempate de arestas exigido na especificação da func. 12)
            int adota = 0;
            if (peso < dist[v]) {
                adota = 1;
            } else if (peso == dist[v] && pai[v] != -1 &&
                       strcmp(g->vertices[u].nomeEstacao,
                              g->vertices[pai[v]].nomeEstacao) < 0) {
                adota = 1;
            }

            if (adota) {
                dist[v] = peso;
                pai[v] = u;
                pesoAresta[v] = peso;
                // nome referencia a string já alocada em g->vertices[v] (vértice já existe)
                insereHeap(heap, v, peso, g->vertices[v].nomeEstacao);
            }
        }
    }

    liberaHeap(heap);
    free(dist);
    free(naMST);
}

// Percorre a MST em profundidade (pré-ordem) a partir do vértice atual,
// imprimindo cada aresta no formato "pai, filho, distancia". Os filhos de
// cada vértice são visitados em ordem crescente de nome; isso é garantido
// automaticamente percorrendo os índices do vetor de vértices do grafo em
// ordem, já que g->vertices está sempre ordenado crescentemente por nome.
static void dfsMST(Grafo *g, int *pai, int *pesoAresta, int atual) {
    for (int v = 0; v < g->nVertices; v++) {
        if (pai[v] == atual) {
            printf("%s, %s, %d\n",
                   g->vertices[atual].nomeEstacao,
                   g->vertices[v].nomeEstacao,
                   pesoAresta[v]);
            dfsMST(g, pai, pesoAresta, v);
        }
    }
}

// funcionalidade 12: constrói o grafo, calcula a MST a partir da estação de
// origem via Prim e imprime a MST em pré-ordem (DFS) a partir da origem
void funcionalidade12(char *arquivoBin, char *nomeOrigem) {
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

    int **matriz = constroiMatrizNaoDirecionada(g);

    int *pai = malloc(g->nVertices * sizeof(int));
    int *pesoAresta = calloc(g->nVertices, sizeof(int));

    prim(g, matriz, idxOrigem, pai, pesoAresta);
    dfsMST(g, pai, pesoAresta, idxOrigem);

    free(pai);
    free(pesoAresta);
    liberaMatriz(matriz, g->nVertices);
    liberaGrafo(g);
}
