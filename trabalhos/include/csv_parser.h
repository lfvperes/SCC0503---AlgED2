/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <stdio.h>
#include "registro.h"

/* Descarta a linha de cabeçalho do CSV caractere a caractere,
   suportando linhas maiores que TAM_REG_DADOS. */
void descartaCabecalhoCSV(FILE *fp);

/* Lê uma linha do CSV já carregada em 'linha' e preenche a struct registro.
   Campos inteiros nulos são representados por -1.
   Aloca memória para nomeEstacao e nomeLinha — deve ser liberada após o uso. */
void lerRegistroCSV(char *linha, struct registro *dados);

#endif // CSV_PARSER_H
