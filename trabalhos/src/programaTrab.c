/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdio.h>
#include <stdlib.h>
#include "escrita.h"
#include "leitura.h"
#include "criaIndice.h"

int main() {
    int funcionalidade;
    char argv[2][100];

    scanf("%d", &funcionalidade);

    int extra = (funcionalidade == 2) ? 1 : 2;
    for (int i = 0; i < extra; i++) {
        scanf("%99s", argv[i]);
    }

    switch (funcionalidade) {
        case 1:
            criaTabela(argv[0], argv[1]);
            break;
        case 2:
            listaTabelaFiltro(argv[0], 0);
            break;
        case 3:
            listaTabelaFiltro(argv[0], atoi(argv[1]));
            break;
        case 4:
            acessoRRN(argv[0], atoi(argv[1]));
            break;
        case 5:
            criaIndice(argv[0], argv[1]);
            break;
    }

    return 0;
}
