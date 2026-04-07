/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registro.h"

#define MAX_LINHA 1024 // numero maximo de caracteres na entrada
#define MAX_ITENS 100


// buffer global para o registro de cabeçalho do arquivo binário
char bufferCabecalho[TAM_REG_CABECALHO];

void ScanQuoteString(char *str);
char* formataSeNulo(int valor);
struct registro leRegistro(FILE *fpBin, int offset);

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

    // NULO no input vira "" pelo ScanQuoteString — corresponde ao sentinela -1
    int valorEhNulo = (valor[0] == '\0') || (strcmp(valor, "NULO") == 0);
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
    int valorComparar = valorEhNulo ? -1 : (int)strtol(valor, NULL, 10);
    return valorLido == valorComparar;
}

// percorre os registros do arquivo duas vezes: primeiro conta os que satisfazem
// todos os m filtros, depois aloca e preenche o array de resultados.
// retorna NULL se algum campo for desconhecido.
struct registro* buscaRegistros(FILE *fpBin, char **nomeCampo, char **valorCampo, int m, int nroRegistros, int *encontrados) {
    *encontrados = 0;

    // passe 1: conta os registros que passam no filtro
    for (int i = 0; i < nroRegistros; i++) {
        int offset = TAM_REG_CABECALHO + i * TAM_REG_DADOS;

        // ignora registros removidos
        char removido;
        fseek(fpBin, offset + OFFSET_REMOVIDO, SEEK_SET);
        fread(&removido, sizeof(char), 1, fpBin);
        if (removido == '1') continue;

        int satisfazTodos = 1; // assume que o registro passa até provar o contrário

        for (int j = 0; j < m; j++) {
            CampoRegistro campo = nomeCampoParaEnum(nomeCampo[j]);
            if (campo == -1) {
                fprintf(stderr, "Campo desconhecido: %s\n", nomeCampo[j]);
                return NULL;
            }

            if (!verificaCampo(fpBin, offset, campo, valorCampo[j])) {
                satisfazTodos = 0;
                break; // AND: basta um falhar para descartar o registro
            }
        }

        if (satisfazTodos) 
            (*encontrados)++;
    }

    if (*encontrados == 0)
        return NULL;

    // passe 2: aloca e preenche o array com os registros encontrados
    struct registro *resultado = malloc(*encontrados * sizeof(struct registro));
    int idx = 0;

    for (int i = 0; i < nroRegistros; i++) {
        int offset = TAM_REG_CABECALHO + i * TAM_REG_DADOS;

        // ignora registros removidos
        char removido;
        fseek(fpBin, offset + OFFSET_REMOVIDO, SEEK_SET);
        fread(&removido, sizeof(char), 1, fpBin);
        if (removido == '1') continue;

        int satisfazTodos = 1; // assume que o registro passa até provar o contrário

        for (int j = 0; j < m; j++) {
            CampoRegistro campo = nomeCampoParaEnum(nomeCampo[j]);
            if (!verificaCampo(fpBin, offset, campo, valorCampo[j])) {
                satisfazTodos = 0;
                break; // AND: basta um falhar para descartar o registro
            }
        }

        if (satisfazTodos) {
            resultado[idx++] = leRegistro(fpBin, offset);
        }
    }

    return resultado;
}

// lê o registro na posição offset do arquivo e retorna um struct registro preenchido
struct registro leRegistro(FILE *fpBin, int offset) {
    struct registro reg;
    fseek(fpBin, offset, SEEK_SET);
    
    // lê o marcador de remoção lógica
    fread(&reg.removido, sizeof(char), 1, fpBin);
    // lê o ponteiro para o próximo registro na lista de removidos
    fread(&reg.proximo, sizeof(int), 1, fpBin);
    // lê os campos de tamanho fixo
    fread(&reg.codEstacao, sizeof(int), 1, fpBin);
    fread(&reg.codLinha, sizeof(int), 1, fpBin);
    fread(&reg.codProxEstacao, sizeof(int), 1, fpBin);
    fread(&reg.distProxEstacao, sizeof(int), 1, fpBin);
    fread(&reg.codLinhaIntegra, sizeof(int), 1, fpBin);
    fread(&reg.codEstIntegra, sizeof(int), 1, fpBin);
    
    // lê o nome da estação: primeiro o tamanho, depois a string
    fread(&reg.tamNomeEstacao, sizeof(int), 1, fpBin);
    reg.nomeEstacao = malloc(reg.tamNomeEstacao + 1);
    fread(reg.nomeEstacao, reg.tamNomeEstacao, 1, fpBin);
    reg.nomeEstacao[reg.tamNomeEstacao] = '\0';
    
    // lê o nome da linha: primeiro o tamanho, depois a string
    fread(&reg.tamNomeLinha, sizeof(int), 1, fpBin);
    reg.nomeLinha = malloc(reg.tamNomeLinha + 1);
    fread(reg.nomeLinha, reg.tamNomeLinha, 1, fpBin);
    reg.nomeLinha[reg.tamNomeLinha] = '\0';

    // descarta bytes de padding para manter alinhamento no tamanho fixo do registro
    int restante = TAM_REG_DADOS - sizeof(char) - 9 * sizeof(int) - reg.tamNomeEstacao - reg.tamNomeLinha;
    char lixo[restante];
    fread(lixo, sizeof(char), restante, fpBin);

    return reg;
}

// imprime os campos de um registro formatados em uma linha
void imprimeRegistro(struct registro reg) {
    char buffSaida[TAM_REG_DADOS];
    int pos = 0;    // posição atual de escrita no buffer de saída
    
    // monta a linha de saída no buffer, usando formataSeNulo para campos opcionais
    pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%d ", reg.codEstacao);
    pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%s ", reg.nomeEstacao);
    pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%d ", reg.codLinha);
    pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%s ", reg.nomeLinha);
    pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%s ", formataSeNulo(reg.codProxEstacao));
    pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%s ", formataSeNulo(reg.distProxEstacao));
    pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%s ", formataSeNulo(reg.codLinhaIntegra));
    pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%s", formataSeNulo(reg.codEstIntegra));
    
    // imprime o registro completo 
    printf("%s\n", buffSaida);
}

// lê arquivo binário e imprime registros de dados formatados
void imprimeTabela(char *arquivoEntrada) {
    struct registro dados;
    FILE *fpBin = fopen(arquivoEntrada, "rb");
    char buffSaida[TAM_REG_DADOS]; // buffer para montar a linha de saída de cada registro

    
    if (fpBin == NULL) {
        printf("Falha no processamento do arquivo.");
        return;
    }

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

    // verifica consistencia
    if (status == '0') {
        printf("Falha no processamento do arquivo.");
        return;
    }

    // itera sobre registros existentes
    for (int i = 0; i < proxRRN; i++) {
        int offset = TAM_REG_CABECALHO + i * TAM_REG_DADOS;
        struct registro dados = leRegistro(fpBin, offset);

        if (dados.removido == '1') {
            free(dados.nomeEstacao);
            free(dados.nomeLinha);
            continue;
        }
        
        imprimeRegistro(dados);
        
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

int listaTabelaFiltro(char *arquivoEntrada, int n) {
    char bufferLinha[MAX_LINHA];    // buffer para a entrada
    int m, encontrados;
    FILE *fpBin = fopen(arquivoEntrada, "rb");    
    
    if (fpBin == NULL) {
        printf("Falha no processamento do arquivo.");
        return 1;
    }

    // lê cabeçalho para extrair proxRRN
    fread(bufferCabecalho, TAM_REG_CABECALHO, 1, fpBin);
    size_t offsetStatus = 0;
    char status = *(char *)(bufferCabecalho + offsetStatus);
    int proxRRN = *(int *)(bufferCabecalho + sizeof(char) + sizeof(int));

    // verifica consistencia
    if (status == '0') {
        printf("Falha no processamento do arquivo.");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        
        // lê m direto do stdin
        scanf("%d", &m);

        // aloca m ponteiros para strings (os nomes)
        char **nomeCampo = malloc(m * sizeof(char *));

        // aloca m ponteiros para os valores
        char **valorCampo = malloc(m * sizeof(char *));

        for (int j = 0; j < m; j++) {

            // lê nome do campos
            scanf("%s", bufferLinha);
            // aloca o necessário e copia
            nomeCampo[j] = malloc(strlen(bufferLinha) + 1);
            strcpy(nomeCampo[j], bufferLinha);

            // lê valor do campo
            // usa ScanQuoteString para campos string, scanf para os demais
            if (strcmp(nomeCampo[j], "nomeEstacao") == 0 || strcmp(nomeCampo[j], "nomeLinha") == 0) {
                ScanQuoteString(bufferLinha);
            } else {
                scanf("%s", bufferLinha);
            }
            // aloca o necessário e copia com +1 para \0
            valorCampo[j] = malloc(strlen(bufferLinha) + 1);
            strcpy(valorCampo[j], bufferLinha);
        }

        // busca registros que satisfazem todos os filtros
        struct registro *resultado = buscaRegistros(fpBin, nomeCampo, valorCampo, m, proxRRN, &encontrados);

        if (resultado == NULL && encontrados == 0) {
            printf("Registro inexistente.\n");
        } else if (resultado == NULL) {
            fprintf(stderr, "Erro na busca.\n");
        } else {
            // imprime e libera cada registro encontrado
            for (int j = 0; j < encontrados; j++) {
                imprimeRegistro(resultado[j]);
                free(resultado[j].nomeEstacao);
                free(resultado[j].nomeLinha);
            }
            free(resultado);
        }

        // libera strings individuais
        for (int j = 0; j < m; j++) {
            free(nomeCampo[j]);
            free(valorCampo[j]);
        }

        // libera o array de ponteiros
        free(nomeCampo);
        free(valorCampo);        

        if (i < n-1)
            printf("\n");
    }

    fclose(fpBin);
    return 0;
}

int acessoRRN(char *arquivoEntrada, int RRN) {
    
    FILE *fpBin = fopen(arquivoEntrada, "rb");

    int offset = TAM_REG_CABECALHO + RRN * TAM_REG_DADOS;
    struct registro dados = leRegistro(fpBin, offset);

    if (dados.removido == '1') {
        free(dados.nomeEstacao);
        free(dados.nomeLinha);
    }
    
    imprimeRegistro(dados);
    
    free(dados.nomeEstacao);
    free(dados.nomeLinha);

    return 0;
}