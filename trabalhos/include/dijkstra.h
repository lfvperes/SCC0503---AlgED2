/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "grafo.h"

// executa o algoritmo de Dijkstra a partir de idxOrigem, preenchendo:
// dist[i]  = menor distância conhecida da origem até o vértice i (INT_MAX se inalcançável)
// pred[i]  = índice do predecessor de i no caminho mínimo (-1 se não houver)
// dist e pred devem ser vetores já alocados pelo chamador, de tamanho g->nVertices
void dijkstra(Grafo *g, int idxOrigem, int *dist, int *pred);

// funcionalidade 11: constrói o grafo, executa Dijkstra entre nomeOrigem e
// nomeDestino e imprime o caminho mínimo (quantidade de estações, distância
// e sequência de nomes)
void funcionalidade11(char *arquivoBin, char *nomeOrigem, char *nomeDestino);

#endif // DIJKSTRA_H
