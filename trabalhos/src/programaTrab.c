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
#include "atualizaRegistro.h"
#include "geraGrafo.h"
#include "ciclos.h"

int main() {
    int funcionalidade;
    char argv[3][100];

    scanf("%d", &funcionalidade);

    int extra = (funcionalidade == 2 || funcionalidade == 10) ? 1 : 2;
    for (int i = 0; i < extra; i++) {
        scanf("%99s", argv[i]);
    }

    // funcionalidades 6 e 7 requerem um terceiro argumento inteiro (n buscas)
    if (funcionalidade >= 6 && funcionalidade < 10) {
        scanf("%99s", argv[2]);
    }
    // f11, f12, f13: le arquivo + token fixo + string com aspas
    if (funcionalidade == 12 || funcionalidade == 13) {
        ScanQuoteString(argv[2]);
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
            buscaIndice(argv[0], argv[1], atoi(argv[2]));
            break;
        case 7:
            removeRegistros(argv[0], argv[1], atoi(argv[2]));
            break;
        case 8:
            insertRegistro(argv[0], argv[1], atoi(argv[2]));
            break;
        case 9:
            atualizaRegistros(argv[0], argv[1], atoi(argv[2]));
            break;
        case 10:
            funcionalidade10(argv[0]);
            break;
        case 13:
            funcionalidade13(argv[0], argv[2]);
            break;
    }

    return 0;
}
