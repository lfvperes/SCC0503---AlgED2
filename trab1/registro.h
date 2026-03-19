// registro.h
#ifndef REGISTRO_H
#define REGISTRO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants
#define TAM_REG_CABECALHO 17
#define TAM_REG_DADOS 80

// Struct definition for a data record
struct registro {
    char removido;
    int proximo;
    int codEstacao;
    int codLinha;
    int codProxEstacao;
    int distProxEstacao;
    int codLinhaIntegra;
    int codEstIntegra;
    int tamNomeEstacao;
    char *nomeEstacao; // This is a pointer, not the actual string data for fixed-size records
    int tamNomeLinha;
    char *nomeLinha;   // This is a pointer, not the actual string data for fixed-size records
};

#endif // REGISTRO_H