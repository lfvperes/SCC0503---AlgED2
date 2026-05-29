/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef REGISTRO_IO_H
#define REGISTRO_IO_H

#include <stdio.h>
#include "registro.h"

/* Lê o registro na posição 'offset' do arquivo binário e retorna
   um struct registro preenchido (nomeEstacao e nomeLinha alocados
   com malloc — devem ser liberados pelo chamador). */
struct registro leRegistro(FILE *fpBin, int offset);

/* Imprime os campos de um registro formatados em uma linha. */
void imprimeRegistro(struct registro reg);

/* Retorna "NULO" se valor == -1, ou o valor convertido para string.
   Usa buffer estático interno — não chamar duas vezes na mesma expressão. */
char* formataSeNulo(int valor);

#endif // REGISTRO_IO_H
