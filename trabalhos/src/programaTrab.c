/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdio.h>
#include <stdlib.h>
#include "escrita.h"
#include "leitura.h"
#include "criaIndice.h"
#include "buscaIndexada.h"
#include "remocao.h"
#include "fornecidas.h"
#include "insereRegistro.h"

int main() {
    int funcionalidade;
    char argv[2][100];

    scanf("%d", &funcionalidade);

    int extra = (funcionalidade == 2) ? 1 : 2;
    for (int i = 0; i < extra; i++) {
        scanf("%99s", argv[i]);
    }

    // funcionalidades 6 e 7 requerem um terceiro argumento inteiro (n buscas)
    int n = 0;
    if (funcionalidade == 6 || funcionalidade == 7 || funcionalidade == 8) {
        scanf("%d", &n);
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
        case 6:
            buscaIndice(argv[0], argv[1], n);
            break;
        case 7:
            removeRegistros(argv[0], argv[1], n);
            break;
        case 8:
            insertRegistro(argv[0], argv[1], n);
            break;
    }

    return 0;
}
