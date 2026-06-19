/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "grafo.h"

// aloca e inicializa um grafo vazio
Grafo *criaGrafo() {
    Grafo *g = malloc(sizeof(Grafo));
    g->nVertices = 0;
    g->capacidade = 16;
    g->vertices = malloc(g->capacidade * sizeof(Vertice));
    return g;
}

// busca um vértice pelo nome usando busca binária (vetor está sempre ordenado)
// retorna o índice ou -1 se não encontrado
int buscaVertice(Grafo *g, char *nome) {
    int esq = 0, dir = g->nVertices - 1;
    while (esq <= dir) {
        int meio = (esq + dir) / 2;
        int cmp = strcmp(g->vertices[meio].nomeEstacao, nome);
        if (cmp == 0) return meio;
        if (cmp < 0)  esq = meio + 1;
        else          dir = meio - 1;
    }
    return -1;
}

// insere novo vértice mantendo o vetor ordenado por nomeEstacao
void insereVertice(Grafo *g, char *nome) {
    // cresce o vetor se necessário
    if (g->nVertices == g->capacidade) {
        g->capacidade *= 2;
        g->vertices = realloc(g->vertices, g->capacidade * sizeof(Vertice));
    }

    // encontra a posição de inserção para manter a ordem crescente
    int pos = g->nVertices;
    while (pos > 0 && strcmp(g->vertices[pos - 1].nomeEstacao, nome) > 0) {
        g->vertices[pos] = g->vertices[pos - 1];
        pos--;
    }

    // preenche o novo vértice na posição encontrada
    g->vertices[pos].nomeEstacao = malloc(strlen(nome) + 1);
    strcpy(g->vertices[pos].nomeEstacao, nome);
    g->vertices[pos].lista = NULL;

    g->nVertices++;
}