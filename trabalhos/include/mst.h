/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef MST_H
#define MST_H

#include "grafo.h"

// funcionalidade 12: constrói o grafo a partir do arquivo binário, calcula a
// árvore geradora mínima (MST) via algoritmo de Prim a partir da estação de
// origem informada (tratando as linhas como bidirecionais) e imprime a MST
// percorrendo-a em profundidade (pré-ordem) a partir da origem.
void funcionalidade12(char *arquivoBin, char *nomeOrigem);

#endif // MST_H
