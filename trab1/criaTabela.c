#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registro.h"

void BinarioNaTela(char *arquivo);

// buffer global para o registro de cabeçalho do arquivo binário
extern char bufferCabecalho[TAM_REG_CABECALHO];

// serializa um registro de dados em um buffer de tamanho fixo (TAM_REG_DADOS)
// e escreve no arquivo binário. campos de tamanho variável são precedidos
// pelo seu tamanho, bytes restantes são preenchidos com $
static void escreveRegistroDados(FILE *fpBin, const struct registro *dados) {
    
    char bufferDados[TAM_REG_DADOS];
    memset(bufferDados, 0, TAM_REG_DADOS);

    size_t offset = 0; // precisa começar no proxRRN

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
            if (n == 0) {
                fseek(fpBin, TAM_REG_CABECALHO + nroRegistros * TAM_REG_DADOS, SEEK_SET);
                return 1;
            }
        }
    }

    // restaura a posição do arquivo para o fim dos registros escritos
    fseek(fpBin, TAM_REG_CABECALHO + nroRegistros * TAM_REG_DADOS, SEEK_SET);
    return 0;
}

// inicializa ou atualiza o registro de cabeçalho no arquivo binário.
// sempre busca o início do arquivo antes de escrever
static int escreveRegistroCabecalho(FILE *fpBin, char status, int proxRRN, int nroEstacoes, int nroParesEstacao) {
    fseek(fpBin, 0, SEEK_SET);

    // offsets de cada campo dentro do buffer de cabeçalho
    size_t offsetStatus = 0;
    size_t offsetTopo = sizeof(char);
    size_t offsetProxRRN = offsetTopo + sizeof(int);             // 5
    size_t offsetNroEstacoes = offsetTopo + 2 * sizeof(int);     // 9
    size_t offsetNroParesEstacao = offsetTopo + 3 * sizeof(int); // 13

    memset(bufferCabecalho, 0, TAM_REG_CABECALHO);
    *(char *)(bufferCabecalho + offsetStatus) = status;
    *(int *)(bufferCabecalho + offsetTopo) = -1;
    *(int *)(bufferCabecalho + offsetProxRRN) = proxRRN;
    *(int *)(bufferCabecalho + offsetNroEstacoes) = nroEstacoes;
    *(int *)(bufferCabecalho + offsetNroParesEstacao) = nroParesEstacao;

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
    token = strsep(&ptr, "\n");
    dados->codEstIntegra = (strlen(token) == 0) ? -1 : atoi(token);

    // campos de controle: não removido e sem próximo
    dados->removido = '0';
    dados->proximo = -1;
}

static void imprimeBinario(char *estacoesBin) {
    struct registro dados;
    FILE *fpBin = fopen(estacoesBin, "rb");

    // ler o cabecalho para imprimir
    fread(bufferCabecalho, TAM_REG_CABECALHO, 1, fpBin);
    
    // offsets de cada campo dentro do buffer de cabeçalho
    size_t offsetStatus = 0;
    size_t offsetTopo = sizeof(char);
    size_t offsetProxRRN = offsetTopo + sizeof(int);             // 5
    size_t offsetNroEstacoes = offsetTopo + 2 * sizeof(int);     // 9
    size_t offsetNroParesEstacao = offsetTopo + 3 * sizeof(int); // 13

    char status = *(char *)(bufferCabecalho + offsetStatus);
    int topo = *(int *)(bufferCabecalho + offsetTopo);
    int proxRRN = *(int *)(bufferCabecalho + offsetProxRRN);
    int nroEstacoes = *(int *)(bufferCabecalho + offsetNroEstacoes);
    int nroParesEstacao = *(int *)(bufferCabecalho + offsetNroParesEstacao);
    
    // imprimindo cabecalho
    printf("%c ", status);
    printf("%d ", topo);
    printf("%d ", proxRRN);
    printf("%d ", nroEstacoes);
    printf("%d\n", nroParesEstacao);

    // imprimindo registros de dados
    for (int i = 0; i < proxRRN; i++) {
        fread(&dados.removido, sizeof(dados.removido), 1, fpBin);
        printf("%c ", dados.removido);
        
        fread(&dados.proximo, sizeof(dados.proximo), 1, fpBin);
        printf("%d ", dados.proximo);
        
        fread(&dados.codEstacao, sizeof(dados.codEstacao), 1, fpBin);
        printf("%3d ", dados.codEstacao);
        
        fread(&dados.codLinha, sizeof(dados.codLinha), 1, fpBin);
        printf("%2d ", dados.codLinha);
        
        fread(&dados.codProxEstacao, sizeof(dados.codProxEstacao), 1, fpBin);
        printf("%3d ", dados.codProxEstacao);
        
        fread(&dados.distProxEstacao, sizeof(dados.distProxEstacao), 1, fpBin);
        printf("%5d ", dados.distProxEstacao);
        
        fread(&dados.codLinhaIntegra, sizeof(dados.codLinhaIntegra), 1, fpBin);
        printf("%2d ", dados.codLinhaIntegra);
        
        fread(&dados.codEstIntegra, sizeof(dados.codEstIntegra), 1, fpBin);
        printf("%3d ", dados.codEstIntegra);
        
        fread(&dados.tamNomeEstacao, sizeof(dados.tamNomeEstacao), 1, fpBin);
        printf("%2d ", dados.tamNomeEstacao);
        
        dados.nomeEstacao = malloc(dados.tamNomeEstacao + 1);
        fread(dados.nomeEstacao, dados.tamNomeEstacao, 1, fpBin);
        // adiciona \0 para poder printar
        dados.nomeEstacao[dados.tamNomeEstacao] = '\0';
        printf("%s ", dados.nomeEstacao);
        free(dados.nomeEstacao);
        
        fread(&dados.tamNomeLinha, sizeof(dados.tamNomeLinha), 1, fpBin);
        printf("%2d ", dados.tamNomeLinha);
        
        dados.nomeLinha = malloc(dados.tamNomeLinha + 1);
        fread(dados.nomeLinha, dados.tamNomeLinha, 1, fpBin);
        // adiciona \0 para poder printar
        dados.nomeLinha[dados.tamNomeLinha] = '\0';
        printf("%s ", dados.nomeLinha);
        free(dados.nomeLinha);
        
        // printf("\niter %d: tamNomeEstacao=%d tamNomeLinha=%d\n", i, dados.tamNomeEstacao, dados.tamNomeLinha);
        int restante = TAM_REG_DADOS - sizeof(char) - 9 * sizeof(int) - dados.tamNomeEstacao - dados.tamNomeLinha;
        char lixo[restante+1];
        fread(lixo, sizeof(char), restante, fpBin);
        lixo[restante] = '\0';
        printf("%s\n", lixo);

    }

    fclose(fpBin);
}

// ordena par para normalizacao
void normalizaPar(struct parEstacao *par) {
    int aux;
    if (par->codEstacao1 > par->codEstacao2) {
        aux = par->codEstacao1;
        par->codEstacao1 = par->codEstacao2;
        par->codEstacao2 = aux;
    }
}

// retorna 1 se pares sao equivalentes depois de normalizar
int paresSaoEquivalentes(struct parEstacao parDisco, struct parEstacao parNovo) {
    normalizaPar(&parDisco);
    normalizaPar(&parNovo);
    if (parDisco.codEstacao1 == parNovo.codEstacao1 && parDisco.codEstacao2 == parNovo.codEstacao2)
        return 1;
    return 0;
}

int parExiste(FILE *fpBin, struct parEstacao parNovo, int nroRegistros) {
    int codEstacaoDisco, codProxEstacaoDisco, offset;
    int offsetCodEstacao = sizeof(char) + sizeof(int);
    int offsetCodProxEstacao = offsetCodEstacao + sizeof(int) + sizeof(int);
    struct parEstacao parDisco;

    for (int i = 0; i < nroRegistros; i++) {
        // le codEstacao do disco
        offset = TAM_REG_CABECALHO + i * TAM_REG_DADOS + offsetCodEstacao;
        fseek(fpBin, offset, SEEK_SET);
        fread(&codEstacaoDisco, sizeof(int), 1, fpBin);
        
        // le codProxEstacao do disco
        offset = TAM_REG_CABECALHO + i * TAM_REG_DADOS + offsetCodProxEstacao;
        fseek(fpBin, offset, SEEK_SET);
        fread(&codProxEstacaoDisco, sizeof(int), 1, fpBin);
        
        // armazena codigos lidos
        parDisco.codEstacao1 = codEstacaoDisco;
        parDisco.codEstacao2 = codProxEstacaoDisco;

        // compara com par a ser inserido
        if (paresSaoEquivalentes(parDisco, parNovo)) {
            fseek(fpBin, TAM_REG_CABECALHO + nroRegistros * TAM_REG_DADOS, SEEK_SET);
            return 1;
        }
    }
    // se nenhum par equivalente foi encontrado
    fseek(fpBin, TAM_REG_CABECALHO + nroRegistros * TAM_REG_DADOS, SEEK_SET);
    return 0;
}

// lê registros do arquivo CSV e escreve no arquivo binário
// o cabeçalho é atualizado após cada registro escrito para manter
// consistência em caso de falha. ao final marca o arquivo como consistente
int criaTabela(char *estacoesCSV, char *estacoesBin) {
    struct registro dados;
    struct parEstacao parNovo;
    int nroRegistros = 0, nroEstacoes = 0, nroParesEstacao = 0;

    FILE *fpCSV = fopen(estacoesCSV, "r");
    FILE *fpBin = fopen(estacoesBin, "wb+");

    if (fpCSV == NULL) {
        printf("Falha no processamento do arquivo.");
        return 1;
    }

    if (fpBin == NULL) {
        perror("Erro ao abrir arquivo binario para escrita");
        return 1;
    }

    // escreve cabeçalho inicial com status '0' (inconsistente) e contadores zerados
    escreveRegistroCabecalho(fpBin, '0', 0, 0, 0);

    char linha[TAM_REG_DADOS];

    // descarta a linha de cabeçalho do CSV, que pode ser maior que TAM_REG_DADOS
    int c;
    while ((c = fgetc(fpCSV)) != '\n' && c != EOF);

    while (fgets(linha, TAM_REG_DADOS, fpCSV)) {
        linha[strcspn(linha, "\r\n")] = '\0';
        lerRegistroCSV(linha, &dados);

        // incrementa nroEstacoes apenas se o nome da estação ainda não foi registrado
        if (!nomeEstacaoExiste(fpBin, &dados, nroRegistros))
            nroEstacoes++;

        // incrementa nroParesEstacao apenas se o par ainda nao foi registrado
        parNovo.codEstacao1 = dados.codEstacao;
        parNovo.codEstacao2 = dados.codProxEstacao;
        if (dados.codProxEstacao != -1 && !parExiste(fpBin, parNovo, nroRegistros))
            nroParesEstacao++;

        escreveRegistroDados(fpBin, &dados);
        nroRegistros++;

        // atualiza o cabeçalho após cada escrita
        escreveRegistroCabecalho(fpBin, '0', nroRegistros, nroEstacoes, nroParesEstacao);

        free(dados.nomeEstacao);
        free(dados.nomeLinha);
    }

    // marca o arquivo como consistente ao final da escrita
    escreveRegistroCabecalho(fpBin, '1', nroRegistros, nroEstacoes, nroParesEstacao);

    fclose(fpCSV);
    fclose(fpBin);

    // imprimeBinario(estacoesBin);
    BinarioNaTela(estacoesBin);

    return 0;
}