/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef BUSCA_INDEXADA_H
#define BUSCA_INDEXADA_H

#include "criaIndice.h"

// busca registros usando o índice primário quando o campo de busca for
// codEstacao, ou busca sequencial para os demais campos.
// executa n buscas lidas da entrada padrão.
// retorna 0 em sucesso, 1 em erro.
int buscaIndice(char *arquivoEntrada, char *arquivoIndice, int n);
ParIndice *carregaIndice(char *arquivoIndice, int *total);
int buscaBinariaIndice(ParIndice *pares, int total, int chave);

#endif // BUSCA_INDEXADA_H