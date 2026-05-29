/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef FILTRO_H
#define FILTRO_H

#include <stdio.h>
#include "registro.h"

/* Converte o nome em string de um campo para o enum CampoRegistro.
   Retorna -1 se o campo for desconhecido. */
CampoRegistro nomeCampoParaEnum(char *nome);

/* Lê o campo indicado do registro na posição offsetRegistro e compara
   com valor. Retorna 1 se bate, 0 caso contrário. */
int verificaCampo(FILE *fpBin, int offsetRegistro,
                  CampoRegistro campo, char *valor);

/* Percorre os registros do arquivo e retorna um array alocado com os
   que satisfazem todos os m filtros (lógica AND).
   *encontrados é preenchido com a quantidade de registros encontrados.
   Retorna NULL se nenhum registro for encontrado ou se algum campo
   for desconhecido (neste último caso *encontrados != 0). */
struct registro* buscaRegistros(FILE *fpBin, char **nomeCampo,
                                char **valorCampo, int m,
                                int nroRegistros, int *encontrados);

#endif // FILTRO_H
