#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registro.h"

// buffer global para o registro de cabeçalho do arquivo binário
char bufferCabecalho[TAM_REG_CABECALHO];

char* formataSeNulo(int valor);

CampoRegistro nomeCampoParaEnum(char *nome) {
    if (strcmp(nome, "removido") == 0)        return CAMPO_REMOVIDO;
    if (strcmp(nome, "proximo") == 0)         return CAMPO_PROXIMO;
    if (strcmp(nome, "codEstacao") == 0)      return CAMPO_COD_ESTACAO;
    if (strcmp(nome, "codLinha") == 0)        return CAMPO_COD_LINHA;
    if (strcmp(nome, "codProxEstacao") == 0)  return CAMPO_COD_PROX_ESTACAO;
    if (strcmp(nome, "distProxEstacao") == 0) return CAMPO_DIST_PROX_ESTACAO;
    if (strcmp(nome, "codLinhaIntegra") == 0) return CAMPO_COD_LINHA_INTEGRA;
    if (strcmp(nome, "codEstIntegra") == 0)   return CAMPO_COD_EST_INTEGRA;
    if (strcmp(nome, "nomeEstacao") == 0)     return CAMPO_NOME_ESTACAO;
    if (strcmp(nome, "nomeLinha") == 0)       return CAMPO_NOME_LINHA;
    return -1; // campo desconhecido
}

// Lê o campo indicado do registro na posição offsetRegistro e compara com valor.
// Retorna 1 se o valor bate, 0 caso contrário.
int verificaCampo(FILE *fpBin, int offsetRegistro, CampoRegistro campo, char *valor) {
    int valorLido;
    int campoOffset;

    switch (campo) {
        // campos inteiros fixos: apenas define o offset e cai na lógica comum abaixo
        case CAMPO_PROXIMO:          
            campoOffset = OFFSET_PROXIMO;          
            break;
        case CAMPO_COD_ESTACAO:      
            campoOffset = OFFSET_COD_ESTACAO;      
            break;
        case CAMPO_COD_LINHA:        
            campoOffset = OFFSET_COD_LINHA;        
            break;
        case CAMPO_COD_PROX_ESTACAO: 
            campoOffset = OFFSET_COD_PROX_ESTACAO; 
            break;
        case CAMPO_DIST_PROX_ESTACAO:
            campoOffset = OFFSET_DIST_PROX_ESTACAO;
            break;
        case CAMPO_COD_LINHA_INTEGRA:
            campoOffset = OFFSET_COD_LINHA_INTEGRA;
            break;
        case CAMPO_COD_EST_INTEGRA:  
            campoOffset = OFFSET_COD_EST_INTEGRA;  
            break;

        case CAMPO_REMOVIDO: {
            char removidoLido;
            fseek(fpBin, offsetRegistro + OFFSET_REMOVIDO, SEEK_SET);
            fread(&removidoLido, sizeof(char), 1, fpBin);
            return removidoLido == valor[0]; // compara char diretamente com valor[0]
        }

        case CAMPO_NOME_ESTACAO: {
            int tamNomeEstacao;
            fseek(fpBin, offsetRegistro + OFFSET_TAM_NOME_ESTACAO, SEEK_SET);
            fread(&tamNomeEstacao, sizeof(int), 1, fpBin);

            char *nomeEstacao = malloc(tamNomeEstacao + 1);
            fseek(fpBin, offsetRegistro + OFFSET_NOME_ESTACAO, SEEK_SET);
            fread(nomeEstacao, tamNomeEstacao, 1, fpBin);
            nomeEstacao[tamNomeEstacao] = '\0';

            int resultado = strcmp(nomeEstacao, valor) == 0;
            free(nomeEstacao);
            return resultado;
        }

        case CAMPO_NOME_LINHA: {
            // precisa ler tamNomeEstacao para calcular onde nomeLinha começa
            int tamNomeEstacao;
            fseek(fpBin, offsetRegistro + OFFSET_TAM_NOME_ESTACAO, SEEK_SET);
            fread(&tamNomeEstacao, sizeof(int), 1, fpBin);

            int offsetTamNomeLinha = OFFSET_NOME_ESTACAO + tamNomeEstacao;

            int tamNomeLinha;
            fseek(fpBin, offsetRegistro + offsetTamNomeLinha, SEEK_SET);
            fread(&tamNomeLinha, sizeof(int), 1, fpBin);

            // ponteiro já está na posição certa após ler tamNomeLinha
            char *nomeLinha = malloc(tamNomeLinha + 1);
            fread(nomeLinha, tamNomeLinha, 1, fpBin);
            nomeLinha[tamNomeLinha] = '\0';

            int resultado = strcmp(nomeLinha, valor) == 0;
            free(nomeLinha);
            return resultado;
        }

        // CAMPO_TAM_NOME_ESTACAO e CAMPO_TAM_NOME_LINHA não são filtráveis
        default: return 0;
    }

    // lógica comum aos campos inteiros fixos
    fseek(fpBin, offsetRegistro + campoOffset, SEEK_SET);
    fread(&valorLido, sizeof(int), 1, fpBin);
    return valorLido == (int)strtol(valor, NULL, 10);
}

struct registro* buscaRegistros(FILE *fpBin, char **nomeCampo, char **valorCampo, int m, int nroRegistros, int *encontrados) {
    *encontrados = 0;

    for (int i = 0; i < nroRegistros; i++) {
        int offset = TAM_REG_CABECALHO + i * TAM_REG_DADOS;
        int satisfazTodos = 1; // assume que o registro passa até provar o contrário

        for (int j = 0; j < m; j++) {
            CampoRegistro campo = nomeCampoParaEnum(nomeCampo[j]);
            if (campo == -1)
                return NULL;
            int satisfaz = verificaCampo(fpBin, offset, campo, valorCampo[j]);

            if (!satisfaz) {
                satisfazTodos = 0;
                break; // AND: basta um falhar para descartar o registro
            }
        }

        if (satisfazTodos) {
            // aqui vamos adicionar o registro ao resultado
        }
    }
}

// lê arquivo binário e imprime registros de dados formatados
void imprimeTabela(char *arquivoEntrada) {
    struct registro dados;
    FILE *fpBin = fopen(arquivoEntrada, "rb");
    char buffSaida[TAM_REG_DADOS]; // buffer para montar a linha de saída de cada registro
    int pos = 0; // posição atual de escrita no buffer de saída

    // lê cabeçalho do arquivo para extrair metadados
    fread(bufferCabecalho, TAM_REG_CABECALHO, 1, fpBin);
    
    // calcula os offsets de cada campo do buffer de cabeçalho
    size_t offsetStatus = 0;
    size_t offsetTopo = sizeof(char);
    size_t offsetProxRRN = offsetTopo + sizeof(int);             // 5
    size_t offsetNroEstacoes = offsetTopo + 2 * sizeof(int);     // 9
    size_t offsetNroParesEstacao = offsetTopo + 3 * sizeof(int); // 13

    // extrai campos do cabeçalho 
    char status = *(char *)(bufferCabecalho + offsetStatus);
    int topo = *(int *)(bufferCabecalho + offsetTopo);
    int proxRRN = *(int *)(bufferCabecalho + offsetProxRRN);
    int nroEstacoes = *(int *)(bufferCabecalho + offsetNroEstacoes);
    int nroParesEstacao = *(int *)(bufferCabecalho + offsetNroParesEstacao);

    // itera sobre registros existentes
    for (int i = 0; i < proxRRN; i++) {
        // lê o marcador de remoção lógica
        fread(&dados.removido, sizeof(dados.removido), 1, fpBin);
        if (dados.removido == '1') {
            // registro removido: avança o ponteiro para o próximo registro sem ler os campos
            fseek(fpBin, TAM_REG_DADOS - sizeof(char), SEEK_CUR);
            continue;
        }
        
        // lê o ponteiro para o próximo registro na lista de removidos
        fread(&dados.proximo, sizeof(dados.proximo), 1, fpBin);

        // lê os campos de tamanho fixo
        fread(&dados.codEstacao, sizeof(dados.codEstacao), 1, fpBin);
        fread(&dados.codLinha, sizeof(dados.codLinha), 1, fpBin);
        fread(&dados.codProxEstacao, sizeof(dados.codProxEstacao), 1, fpBin);
        fread(&dados.distProxEstacao, sizeof(dados.distProxEstacao), 1, fpBin);
        fread(&dados.codLinhaIntegra, sizeof(dados.codLinhaIntegra), 1, fpBin);
        fread(&dados.codEstIntegra, sizeof(dados.codEstIntegra), 1, fpBin);
        
        // lê o nome da estação: primeiro o tamanho, depois a string
        fread(&dados.tamNomeEstacao, sizeof(dados.tamNomeEstacao), 1, fpBin);
        dados.nomeEstacao = malloc(dados.tamNomeEstacao + 1);
        fread(dados.nomeEstacao, dados.tamNomeEstacao, 1, fpBin);
        dados.nomeEstacao[dados.tamNomeEstacao] = '\0'; // termina a string manualmente
        
        // lê o nome da linha: primeiro o tamanho, depois a string
        fread(&dados.tamNomeLinha, sizeof(dados.tamNomeLinha), 1, fpBin);
        dados.nomeLinha = malloc(dados.tamNomeLinha + 1);
        fread(dados.nomeLinha, dados.tamNomeLinha, 1, fpBin);
        dados.nomeLinha[dados.tamNomeLinha] = '\0'; // termina a string manualmente
        
        // descarta os bytes de padding para manter o alinhamento no tamanho fixo do registro
        int restante = TAM_REG_DADOS - sizeof(char) - 9 * sizeof(int) - dados.tamNomeEstacao - dados.tamNomeLinha;
        char lixo[restante+1];
        fread(lixo, sizeof(char), restante, fpBin);
        
        // monta a linha de saída no buffer, usando formataSeNulo para campos opcionais
        pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%d ", dados.codEstacao);
        pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%s ", dados.nomeEstacao);
        pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%d ", dados.codLinha);
        pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%s ", dados.nomeLinha);
        pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%s ", formataSeNulo(dados.codProxEstacao));
        pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%s ", formataSeNulo(dados.distProxEstacao));
        pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%s ", formataSeNulo(dados.codLinhaIntegra));
        pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%s", formataSeNulo(dados.codEstIntegra));
        
        // imprime o registro completo e reinicia o buffer para o próximo
        printf("%s\n", buffSaida);
        pos = 0;
        
        free(dados.nomeEstacao);
        free(dados.nomeLinha);
    }

    fclose(fpBin);
}

// retorna "NULO " se o valor for -1 (sentinela para campo ausente),
// ou o valor convertido para string caso contrário
char* formataSeNulo(int valor) {
    if (valor == -1)
        return "NULO";
    static char buf[10];
    snprintf(buf, sizeof(buf), "%d", valor);
    return buf;
}