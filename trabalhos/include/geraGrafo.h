/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef GERA_GRAFO_H
#define GERA_GRAFO_H

#include "grafo.h"

// constrói e retorna o grafo completo a partir do arquivo binário;
// retorna NULL em caso de erro
Grafo *constroiGrafo(char *arquivoBin);

// funcionalidade 10: constrói o grafo e imprime na saída padrão
void funcionalidade10(char *arquivoBin);

#endif // GERA_GRAFO_H