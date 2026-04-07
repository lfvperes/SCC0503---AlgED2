/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdlib.h>
#include <stdio.h>

int criaTabela(char *estacoesCSV, char *estacoesBin);
void imprimeTabela(char *arquivoEntrada);
int listaTabelaFiltro(char *arquivoEntrada, int n);
int acessoRRN(char *arquivoEntrada, int RRN);

int main(){
    int argc = 0;
    char argv[3][100];

    while (argc < 3 && scanf("%99s", argv[argc]) == 1) {
        argc++;
    }
    int funcionalidade = atoi(argv[0]);
    switch(funcionalidade) {
        case 1:
            criaTabela(argv[1], argv[2]);
            break;
        case 2:
            imprimeTabela(argv[1]);
            break;
        case 3:
            listaTabelaFiltro(argv[1], atoi(argv[2]));
            break;
        case 4:
            acessoRRN(argv[1], atoi(argv[2]));
            break;
    }

    return 0;
}
