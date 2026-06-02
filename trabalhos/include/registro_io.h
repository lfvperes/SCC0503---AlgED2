/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef REGISTRO_IO_H
#define REGISTRO_IO_H

#include <stdio.h>
#include "registro.h"

// lê o registro na posição offset do arquivo binário e retorna
// um struct registro preenchido (nomeEstacao e nomeLinha alocados
// com malloc — devem ser liberados pelo chamador)
struct registro leRegistro(FILE *fpBin, int offset);

// lê o registro a partir da posição corrente do cursor, sem fseek
// O cursor deve estar posicionado no início do registro
// nomeEstacao e nomeLinha são alocados com malloc — devem ser liberados pelo chamador
struct registro leRegistroSeq(FILE *fpBin);

// imprime os campos de um registro formatados em uma linha
void imprimeRegistro(struct registro reg);

// retorna "NULO" se valor == -1, ou o valor convertido para string
// usa buffer estático interno — não chamar duas vezes na mesma expressão
char* formataSeNulo(int valor);

#endif // REGISTRO_IO_H
