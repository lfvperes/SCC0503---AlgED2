/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdlib.h>

int criaTabela(char *estacoesCSV, char *estacoesBin);
int listaTabela(char *arquivoEntrada);
int listaTabelaFiltro(char *arquivoEntrada, char *n);
int acessoRRN(char *arquivoEntrada, char *RRN);

int main(int argc, char *argv[]) {

    int funcionalidade = atoi(argv[1]);
    switch(funcionalidade) {
        case 1:
            criaTabela(argv[2], argv[3]);
            break;
        case 2:
            listaTabela(argv[2]);
            break;
        case 3:
            listaTabelaFiltro(argv[2], argv[3]);
            break;
        case 4:
            acessoRRN(argv[2], argv[3]);
            break;
    }

    return 0;
}
