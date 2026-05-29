/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef ESCRITA_H
#define ESCRITA_H

/* Lê registros do arquivo CSV e gera o arquivo binário correspondente.
   O cabeçalho é atualizado após cada registro para consistência.
   Retorna 0 em sucesso, 1 em erro. */
int criaTabela(char *estacoesCSV, char *estacoesBin);

#endif // ESCRITA_H
