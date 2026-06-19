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

// insere linha ordenadamente no array de linhas de uma aresta existente
static void insereLinha(ArestaAdj *aresta, char *nomeLinha) {
    // cresce o array de linhas
    aresta->linhas = realloc(aresta->linhas, (aresta->nLinhas + 1) * sizeof(char *));

    // encontra posição de inserção para manter ordem crescente
    int pos = aresta->nLinhas;
    while (pos > 0 && strcmp(aresta->linhas[pos - 1], nomeLinha) > 0) {
        aresta->linhas[pos] = aresta->linhas[pos - 1];
        pos--;
    }

    aresta->linhas[pos] = malloc(strlen(nomeLinha) + 1);
    strcpy(aresta->linhas[pos], nomeLinha);
    aresta->nLinhas++;
}

// cria um novo nó ArestaAdj com os dados fornecidos
static ArestaAdj *criaArestaAdj(char *nomeV, int dist, char *nomeLinha) {
    ArestaAdj *a = malloc(sizeof(ArestaAdj));
    a->nomeProxEstacao = malloc(strlen(nomeV) + 1);
    strcpy(a->nomeProxEstacao, nomeV);
    a->distancia = dist;
    a->nLinhas = 0;
    a->linhas = NULL;
    a->prox = NULL;
    insereLinha(a, nomeLinha);
    return a;
}

// insere aresta (nomeU → nomeV); se já existir, apenas adiciona nomeLinha
void insereAresta(Grafo *g, char *nomeU, char *nomeV, int dist, char *nomeLinha) {
    int idxU = buscaVertice(g, nomeU);
    if (idxU == -1) return; // vértice origem não existe

    ArestaAdj **atual = &g->vertices[idxU].lista;

    // percorre a lista procurando nomeV ou a posição de inserção ordenada
    while (*atual != NULL) {
        int cmp = strcmp((*atual)->nomeProxEstacao, nomeV);

        if (cmp == 0) {
            // aresta já existe — apenas adiciona a linha
            insereLinha(*atual, nomeLinha);
            return;
        }

        if (cmp > 0) break; // posição de inserção encontrada

        atual = &(*atual)->prox;
    }

    // aresta nova — cria e encadeia na posição correta
    ArestaAdj *nova = criaArestaAdj(nomeV, dist, nomeLinha);
    nova->prox = *atual;
    *atual = nova;
}

// imprime o grafo no formato da funcionalidade 10
void imprimeGrafo(Grafo *g) {
    for (int i = 0; i < g->nVertices; i++) {
        Vertice *v = &g->vertices[i];

        // imprime o nome do vértice
        printf("%s", v->nomeEstacao);

        // imprime cada aresta da lista de adjacências
        ArestaAdj *a = v->lista;
        while (a != NULL) {
            printf(", %s, %d", a->nomeProxEstacao, a->distancia);

            // imprime os nomes das linhas
            for (int j = 0; j < a->nLinhas; j++) {
                printf(", %s", a->linhas[j]);
            }

            a = a->prox;
        }

        printf("\n");
    }
}

// libera toda a memória alocada pelo grafo
void liberaGrafo(Grafo *g) {
    for (int i = 0; i < g->nVertices; i++) {
        free(g->vertices[i].nomeEstacao);

        // libera cada nó da lista de adjacências
        ArestaAdj *a = g->vertices[i].lista;
        while (a != NULL) {
            ArestaAdj *prox = a->prox;
            free(a->nomeProxEstacao);
            for (int j = 0; j < a->nLinhas; j++) {
                free(a->linhas[j]);
            }
            free(a->linhas);
            free(a);
            a = prox;
        }
    }

    free(g->vertices);
    free(g);
}