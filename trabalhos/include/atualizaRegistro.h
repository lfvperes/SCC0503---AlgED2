/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef ATUALIZA_REGISTRO_H
#define ATUALIZA_REGISTRO_H

/* Atualiza registros do arquivo de dados que satisfaçam um critério
   de busca, sobrescrevendo os campos indicados. Mantém o índice
   primário e os contadores do cabeçalho consistentes.
   Retorna 0 em sucesso, 1 em erro. */
int atualizaRegistros(char *arquivoEntrada, char *arquivoIndice, int n);

#endif // ATUALIZA_REGISTRO_H