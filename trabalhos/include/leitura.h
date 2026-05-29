/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef LEITURA_H
#define LEITURA_H

/* Lê o arquivo binário e imprime todos os registros não removidos. */
void imprimeTabela(char *arquivoEntrada);

/* Lê n consultas com filtros da stdin e imprime os registros correspondentes.
   Se n == 0, imprime todos os registros sem aplicar filtros. */
int listaTabelaFiltro(char *arquivoEntrada, int n);

/* Acessa e imprime o registro no RRN informado.
   Retorna 0 em sucesso, 1 em erro ou registro inexistente. */
int acessoRRN(char *arquivoEntrada, int RRN);

#endif // LEITURA_H
