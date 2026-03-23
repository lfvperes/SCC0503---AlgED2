/*
Fernando Barbosa Leite - 14570458
Vamo (coloque o nome pf kk)
*/

#include <stdlib.h>

int func1(char *estacoesCSV, char *estacoesBin);
int func2(char *arquivoEntrada);
int func3(char *arquivoEntrada, char *n);
int func4(char *arquivoEntrada, char *RRN);

int main(int argc, char *argv[]) {

    int funcionalidade = atoi(argv[1]);
    switch(funcionalidade) {
        case 1:
            func1(argv[2], argv[3]);
            break;
        case 2:
            func2(argv[2]);
            break;
        case 3:
            func3(argv[2], argv[3]);
            break;
        case 4:
            func4(argv[2], argv[3]);
            break;
    }

    return 0;
}
