/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef FORNECIDAS_H
#define FORNECIDAS_H

/* Abre o arquivo binário, calcula o checksum e imprime na tela.
   Deve ser chamada após fechar o arquivo com fclose. */
void BinarioNaTela(char *arquivo);

/* Lê um campo string delimitado por aspas (") da stdin.
   Trata campos NULOS (string vazia) e campos sem aspas. */
void ScanQuoteString(char *str);

#endif // FORNECIDAS_H
