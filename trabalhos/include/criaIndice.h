/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef CRIA_INDICE_H
#define CRIA_INDICE_H

typedef struct {
    int codEstacao;
    int rrn;
} ParIndice;

// comparador para qsort: ordena por codEstacao crescente
int comparaPar(const void *a, const void *b);

int criaIndice(char *arquivoEntrada, char *arquivoIndice);

#endif // CRIA_INDICE_H