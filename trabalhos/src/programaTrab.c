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
#include "dijkstra.h"

int main() {
    int funcionalidade;
    // argv[0]: arquivo.bin, argv[1]: arquivo_indice.bin
    // argv[2]: primeiro valor de string (origem para f12/f13, ou origem para f11)
    // argv[3]: segundo valor de string (destino para f11)
    char argv[4][200];

    scanf("%d", &funcionalidade);

    // f2 e f10: so o nome do arquivo binario
    // demais: arquivo.bin + arquivo_indice.bin
    int extra = (funcionalidade == 2 || funcionalidade == 10) ? 1 : 2;
    for (int i = 0; i < extra; i++) {
        scanf("%199s", argv[i]);
    }

    // funcionalidades 6-9: terceiro argumento inteiro (n buscas)
    if (funcionalidade >= 6 && funcionalidade < 10) {
        scanf("%199s", argv[2]);
    }

    // f11: nomeEstacaoOrigem "valorOrigem" nomeEstacaoDestino "valorDestino"
    // consome os dois tokens fixos e lê os dois valores com aspas
    if (funcionalidade == 11) {
        char nomeCampo[200];
        scanf("%199s", nomeCampo); // consome "nomeEstacaoOrigem"
        ScanQuoteString(argv[2]);      // lê "valorOrigem"
        scanf("%199s", nomeCampo); // consome "nomeEstacaoDestino"
        ScanQuoteString(argv[3]);      // lê "valorDestino"
    }

    // f12 e f13: nomeEstacaoOrigem "valorOrigem"
    // consome o token fixo e lê o valor com aspas
    if (funcionalidade == 12 || funcionalidade == 13) {
        char nomeCampo[200];
        scanf("%199s", nomeCampo); // consome "nomeEstacaoOrigem"
        ScanQuoteString(argv[2]);      // lê "valorOrigem"
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
        case 11:
            funcionalidade11(argv[0], argv[2], argv[3]);
            break;
        case 13:
            funcionalidade13(argv[0], argv[2]);
            break;
    }

    return 0;
}
