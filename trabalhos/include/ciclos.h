/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef CICLOS_H
#define CICLOS_H

#include "grafo.h"

// DFS recursivo com backtracking para contar ciclos simples a partir da origem
// idxOrigem é o índice do vertice de origem no vetor de vertices;
// visitado é um array de booleanos indexado pela posição do vertice
void dfsCiclos(Grafo *g, int idxOrigem, int idxAtual, int *visitado, int *count);

// funcionalidade 13: constrói o grafo, localiza a origem e conta ciclos simples
void funcionalidade13(char *arquivoBin, char *nomeOrigem);

#endif // CICLOS_H