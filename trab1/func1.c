#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registro.h"

// buffer global para o registro de cabeçalho do arquivo binário
char bufferCabecalho[TAM_REG_CABECALHO];

// serializa um registro de dados em um buffer de tamanho fixo (TAM_REG_DADOS)
// e escreve no arquivo binário. campos de tamanho variável são precedidos
// pelo seu tamanho, bytes restantes são preenchidos com $
static void escreveRegistroDados(FILE *fpBin, const struct registro *dados) {

    char bufferDados[TAM_REG_DADOS];
    memset(bufferDados, 0, TAM_REG_DADOS);

    size_t offset = 0;

    // campos de tamanho fixo - copiados diretamente com memcpy
    memcpy(bufferDados + offset, &dados->removido, sizeof(dados->removido));
    offset += sizeof(dados->removido);

    memcpy(bufferDados + offset, &dados->proximo, sizeof(dados->proximo));
    offset += sizeof(dados->proximo);

    memcpy(bufferDados + offset, &dados->codEstacao, sizeof(dados->codEstacao));
    offset += sizeof(dados->codEstacao);

    memcpy(bufferDados + offset, &dados->codLinha, sizeof(dados->codLinha));
    offset += sizeof(dados->codLinha);

    memcpy(bufferDados + offset, &dados->codProxEstacao, sizeof(dados->codProxEstacao));
    offset += sizeof(dados->codProxEstacao);

    memcpy(bufferDados + offset, &dados->distProxEstacao, sizeof(dados->distProxEstacao));
    offset += sizeof(dados->distProxEstacao);

    memcpy(bufferDados + offset, &dados->codLinhaIntegra, sizeof(dados->codLinhaIntegra));
    offset += sizeof(dados->codLinhaIntegra);

    memcpy(bufferDados + offset, &dados->codEstIntegra, sizeof(dados->codEstIntegra));
    offset += sizeof(dados->codEstIntegra);

    // campos de tamanho variável - precedidos pelo tamanho em bytes
    memcpy(bufferDados + offset, &dados->tamNomeEstacao, sizeof(dados->tamNomeEstacao));
    offset += sizeof(dados->tamNomeEstacao);

    memcpy(bufferDados + offset, dados->nomeEstacao, dados->tamNomeEstacao);
    offset += dados->tamNomeEstacao;

    memcpy(bufferDados + offset, &dados->tamNomeLinha, sizeof(dados->tamNomeLinha));
    offset += sizeof(dados->tamNomeLinha);

    // nomeLinha pode ser nulo (tam == 0), nesse caso nada é copiado
    if (dados->tamNomeLinha > 0)
        memcpy(bufferDados + offset, dados->nomeLinha, dados->tamNomeLinha);
    offset += dados->tamNomeLinha;

    // bytes restantes do registro são preenchidos com $
    memset(bufferDados + offset, '$', TAM_REG_DADOS - offset);

    if (fwrite(bufferDados, TAM_REG_DADOS, 1, fpBin) != 1) {
        perror("Erro ao escrever registro de dados no arquivo binario");
    }
}

// verifica se um registro com o mesmo nomeEstacao já foi escrito no arquivo binário.
// retorna 1 se encontrado, 0 caso contrário.
static int nomeEstacaoExiste(FILE *fpBin, const struct registro *dados, int nroRegistros) {
    int bufferTamNomeEstacao, n;
    int offset;

    for (int i = 0; i < nroRegistros; i++) {
        // calcula offset absoluto até o campo tamNomeEstacao do registro i,
        // pulando o cabeçalho, os registros anteriores e os 29 bytes de campos fixos
        offset = TAM_REG_CABECALHO + i * TAM_REG_DADOS + 29;
        fseek(fpBin, offset, SEEK_SET);

        // lê o tamanho do nomeEstacao do registro i
        fread(&bufferTamNomeEstacao, sizeof(int), 1, fpBin);

        // só compara os nomes se tiverem o mesmo tamanho
        if (bufferTamNomeEstacao == dados->tamNomeEstacao) {
            // lê o nomeEstacao do registro i para um buffer temporário
            char bufferNomeEstacao[bufferTamNomeEstacao];
            fread(bufferNomeEstacao, bufferTamNomeEstacao, 1, fpBin);

            // compara o nome lido com o nome do registro atual
            n = memcmp(bufferNomeEstacao, dados->nomeEstacao, dados->tamNomeEstacao);
            if (n == 0)
                return 1;
        }
    }

    // restaura a posição do arquivo para o fim dos registros escritos
    fseek(fpBin, TAM_REG_CABECALHO + nroRegistros * TAM_REG_DADOS, SEEK_SET);
    return 0;
}

// inicializa ou atualiza o registro de cabeçalho no arquivo binário.
// sempre busca o início do arquivo antes de escrever
static int escreveRegistroCabecalho(FILE *fpBin, char status, int proxRRN, int nroEstacoes) {
    fseek(fpBin, 0, SEEK_SET);

    // offsets de cada campo dentro do buffer de cabeçalho
    size_t offset_status = 0;
    size_t offset_topo = 1;
    size_t offset_proxRRN = 1 + sizeof(int);             // 5
    size_t offset_nroEstacoes = 1 + 2 * sizeof(int);     // 9
    size_t offset_nroParesEstacao = 1 + 3 * sizeof(int); // 13

    memset(bufferCabecalho, 0, TAM_REG_CABECALHO);
    *(char *)(bufferCabecalho + offset_status) = status;
    *(int *)(bufferCabecalho + offset_topo) = -1;
    *(int *)(bufferCabecalho + offset_proxRRN) = proxRRN;
    *(int *)(bufferCabecalho + offset_nroEstacoes) = nroEstacoes;
    *(int *)(bufferCabecalho + offset_nroParesEstacao) = 0;

    if (fwrite(bufferCabecalho, TAM_REG_CABECALHO, 1, fpBin) != 1) {
        perror("Erro ao escrever cabecalho no arquivo binario");
        return 1;
    }
    return 0;
}

// lê uma linha do CSV já carregada em linha e preenche a struct registro apontada
// por dados. campos inteiros vazios são representados por -1, campos de string
// vazios são representados por tamNome = 0. aloca memória para os campos de string,
// que deve ser liberada após o uso.
static void lerRegistroCSV(char *linha, struct registro *dados) {
    char *ptr = linha;
    char *token;

    // codEstacao - não aceita nulo
    token = strsep(&ptr, ",");
    dados->codEstacao = atoi(token);

    // nomeEstacao - não aceita nulo, aloca e copia o conteúdo do token
    token = strsep(&ptr, ",");
    dados->tamNomeEstacao = strlen(token);
    dados->nomeEstacao = malloc(dados->tamNomeEstacao);
    memcpy(dados->nomeEstacao, token, dados->tamNomeEstacao);

    // codLinha - nulo representado por -1
    token = strsep(&ptr, ",");
    dados->codLinha = (strlen(token) == 0) ? -1 : atoi(token);

    // nomeLinha - nulo representado por tamNomeLinha = 0, aloca e copia se não vazio
    token = strsep(&ptr, ",");
    dados->tamNomeLinha = strlen(token);
    dados->nomeLinha = malloc(dados->tamNomeLinha);
    memcpy(dados->nomeLinha, token, dados->tamNomeLinha);

    // codProxEstacao - nulo representado por -1
    token = strsep(&ptr, ",");
    dados->codProxEstacao = (strlen(token) == 0) ? -1 : atoi(token);

    // distProxEstacao - nulo representado por -1
    token = strsep(&ptr, ",");
    dados->distProxEstacao = (strlen(token) == 0) ? -1 : atoi(token);

    // codLinhaIntegra - nulo representado por -1
    token = strsep(&ptr, ",");
    dados->codLinhaIntegra = (strlen(token) == 0) ? -1 : atoi(token);

    // codEstIntegra - nulo representado por -1
    token = strsep(&ptr, ",");
    dados->codEstIntegra = (strlen(token) == 0) ? -1 : atoi(token);

    // campos de controle: não removido e sem próximo
    dados->removido = '0';
    dados->proximo = -1;
}

// lê registros do arquivo CSV e escreve no arquivo binário
// o cabeçalho é atualizado após cada registro escrito para manter
// consistência em caso de falha. ao final marca o arquivo como consistente
int func1(char *estacoesCSV, char *estacoesBin) {
    struct registro dados;
    int nroRegistros = 0, nroEstacoes = 0;

    FILE *fpCSV = fopen(estacoesCSV, "r");
    FILE *fpBin = fopen(estacoesBin, "wb");

    if (fpBin == NULL) {
        perror("Erro ao abrir arquivo binario para escrita");
        return 1;
    }

    // escreve cabeçalho inicial com status '0' (inconsistente) e contadores zerados
    escreveRegistroCabecalho(fpBin, '0', 0, 0);

    char linha[TAM_REG_DADOS];

    // descarta a linha de cabeçalho do CSV, que pode ser maior que TAM_REG_DADOS
    int c;
    while ((c = fgetc(fpCSV)) != '\n' && c != EOF);

    while (fgets(linha, TAM_REG_DADOS, fpCSV)) {

        lerRegistroCSV(linha, &dados);

        // incrementa nroEstacoes apenas se o nome da estação ainda não foi registrado
        if (!nomeEstacaoExiste(fpBin, &dados, nroRegistros))
            nroEstacoes++;

        escreveRegistroDados(fpBin, &dados);
        nroRegistros++;

        // atualiza o cabeçalho após cada escrita
        escreveRegistroCabecalho(fpBin, '0', nroRegistros, nroEstacoes);

        free(dados.nomeEstacao);
        free(dados.nomeLinha);
    }

    // marca o arquivo como consistente ao final da escrita
    escreveRegistroCabecalho(fpBin, '1', nroRegistros, nroEstacoes);

    fclose(fpCSV);
    fclose(fpBin);
    return 0;
}