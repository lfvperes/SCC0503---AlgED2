/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "heap.h"

// aloca e inicializa um heap vazio
MinHeap *criaHeap() {
    MinHeap *h = malloc(sizeof(MinHeap));
    h->tamanho = 0;
    h->capacidade = 16;
    h->nos = malloc(h->capacidade * sizeof(HeapNode));
    return h;
}

// retorna 1 se o nó a tem prioridade menor (deve ficar mais acima no heap) que o nó b
// prioridade: menor dist primeiro; em empate, menor nome (strcmp) primeiro
static int menorPrioridade(HeapNode a, HeapNode b) {
    if (a.dist != b.dist) return a.dist < b.dist;
    return strcmp(a.nome, b.nome) < 0;
}

// troca dois nós do heap de posição
static void troca(HeapNode *a, HeapNode *b) {
    HeapNode tmp = *a;
    *a = *b;
    *b = tmp;
}

// restaura a propriedade de heap subindo o elemento da posição i enquanto necessário
static void sobe(MinHeap *h, int i) {
    while (i > 0) {
        int pai = (i - 1) / 2;
        if (menorPrioridade(h->nos[i], h->nos[pai])) {
            troca(&h->nos[i], &h->nos[pai]);
            i = pai;
        } else {
            break;
        }
    }
}

// restaura a propriedade de heap descendo o elemento da posição i enquanto necessário
static void desce(MinHeap *h, int i) {
    while (1) {
        int esq = 2 * i + 1;
        int dir = 2 * i + 2;
        int menor = i;

        if (esq < h->tamanho && menorPrioridade(h->nos[esq], h->nos[menor]))
            menor = esq;
        if (dir < h->tamanho && menorPrioridade(h->nos[dir], h->nos[menor]))
            menor = dir;

        if (menor == i) break;

        troca(&h->nos[i], &h->nos[menor]);
        i = menor;
    }
}

// insere um novo nó no heap, mantendo a propriedade de min-heap
void insereHeap(MinHeap *h, int idx, int dist, char *nome) {
    // cresce o vetor se necessário
    if (h->tamanho == h->capacidade) {
        h->capacidade *= 2;
        h->nos = realloc(h->nos, h->capacidade * sizeof(HeapNode));
    }

    h->nos[h->tamanho].idx = idx;
    h->nos[h->tamanho].dist = dist;
    h->nos[h->tamanho].nome = nome;

    sobe(h, h->tamanho);
    h->tamanho++;
}

// remove e retorna o nó de menor prioridade
HeapNode extraiMin(MinHeap *h) {
    HeapNode min = h->nos[0];

    h->tamanho--;
    h->nos[0] = h->nos[h->tamanho];
    desce(h, 0);

    return min;
}

// retorna 1 se o heap estiver vazio, 0 caso contrário
int heapVazio(MinHeap *h) {
    return h->tamanho == 0;
}

// libera a memória alocada pelo heap (não libera os nomes referenciados)
void liberaHeap(MinHeap *h) {
    free(h->nos);
    free(h);
}
