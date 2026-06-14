/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/
#ifndef INSERE_REGISTRO_H
#define INSERE_REGISTRO_H

#include <stdio.h>
#include "criaIndice.h"

int insertRegistro(char *arquivoEntrada, char *arquivoIndice, int n);
int carregaPares(FILE *fpIndice, ParIndice **pares);

#endif