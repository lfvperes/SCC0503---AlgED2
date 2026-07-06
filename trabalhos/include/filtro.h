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

// compara um campo do registro (já em memória) com o valor fornecido como string
// retorna 1 se o campo satisfaz o critério, 0 caso contrário
int verificaCampoMem(const struct registro *reg, CampoRegistro campo, char *valor);

typedef enum { MODO_IMPRIMIR, MODO_COLETAR_RRN } ModoBusca;

// percorre os registros do arquivo aplicando m filtros (lógica AND).
// modo MODO_IMPRIMIR: imprime cada registro encontrado.
// modo MODO_COLETAR_RRN: preenche *rrns (deve ser inicializado como NULL)
//   com array alocado dos RRNs encontrados.
// retorna o número de registros encontrados, ou -1 em caso de erro.
int buscaRegistros(FILE *fpBin, char **nomeCampo, char **valorCampo,
                   int m, int nroRegistros, ModoBusca modo, int **rrns);

#endif // FILTRO_H