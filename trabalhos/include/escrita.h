/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef ESCRITA_H
#define ESCRITA_H

#include "registro.h"

/* Lê registros do arquivo CSV e gera o arquivo binário correspondente.
   O cabeçalho é atualizado após cada registro para consistência.
   Retorna 0 em sucesso, 1 em erro. */
int criaTabela(char *estacoesCSV, char *estacoesBin);

void escreveRegistroDados(FILE *fpBin, const struct registro *dados);
void normalizaPar(struct parEstacao *par);

int nomeEstacaoExiste(char **nomesVistos, int nroNomes, const struct registro *dados);

int parExiste(struct parEstacao *paresVistos,
              int nroPares,
              struct parEstacao parNovo);

int paresSaoEquivalentes(struct parEstacao parDisco,
                         struct parEstacao parNovo);

#endif // ESCRITA_H
