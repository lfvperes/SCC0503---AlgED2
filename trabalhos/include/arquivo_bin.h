/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef ARQUIVO_BIN_H
#define ARQUIVO_BIN_H

#include <stdio.h>
#include "registro.h"

/* Escreve (ou sobrescreve) o registro de cabeçalho no início do arquivo.
   Sempre posiciona no início antes de escrever.
   Retorna 0 em sucesso, 1 em erro. */
int escreveCabecalho(FILE *fpBin, char status, int topo, int proxRRN,
                     int nroEstacoes, int nroParesEstacao);

/* Lê o registro de cabeçalho e preenche os ponteiros fornecidos.
   Sempre posiciona no início antes de ler.
   Retorna 0 em sucesso, 1 em erro. */
int leCabecalho(FILE *fpBin, char *status, int *topo,
                int *proxRRN, int *nroEstacoes, int *nroParesEstacao);

#endif // ARQUIVO_BIN_H
