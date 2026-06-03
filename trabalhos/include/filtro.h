/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef FILTRO_H
#define FILTRO_H

#include <stdio.h>
#include "registro.h"

// converte o nome em string de um campo para o enum CampoRegistro
// retorna -1 se o campo for desconhecido
CampoRegistro nomeCampoParaEnum(char *nome);


// percorre os registros do arquivo e retorna um array alocado com os
// que satisfazem todos os m filtros (lógica AND),
// *encontrados é preenchido com a quantidade de registros encontrados
// retorna NULL se nenhum registro for encontrado ou se algum campo
// for desconhecido (neste último caso *encontrados != 0)
int buscaRegistros(FILE *fpBin, char **nomeCampo, char **valorCampo, int m, int nroRegistros);

#endif // FILTRO_H
