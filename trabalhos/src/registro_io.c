/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registro.h"
#include "registro_io.h"

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
    int restante = TAM_REG_DADOS - sizeof(char) - 9 * sizeof(int)
                   - reg.tamNomeEstacao - reg.tamNomeLinha;
    char lixo[restante];
    fread(lixo, sizeof(char), restante, fpBin);

    return reg;
}

// lê o registro a partir da posição corrente, sem fseek
struct registro leRegistroSeq(FILE *fpBin) {
    struct registro reg;

    fread(&reg.removido, sizeof(char), 1, fpBin);
    fread(&reg.proximo, sizeof(int), 1, fpBin);
    fread(&reg.codEstacao, sizeof(int), 1, fpBin);
    fread(&reg.codLinha, sizeof(int), 1, fpBin);
    fread(&reg.codProxEstacao, sizeof(int), 1, fpBin);
    fread(&reg.distProxEstacao, sizeof(int), 1, fpBin);
    fread(&reg.codLinhaIntegra, sizeof(int), 1, fpBin);
    fread(&reg.codEstIntegra, sizeof(int), 1, fpBin);

    fread(&reg.tamNomeEstacao, sizeof(int), 1, fpBin);
    reg.nomeEstacao = malloc(reg.tamNomeEstacao + 1);
    fread(reg.nomeEstacao, reg.tamNomeEstacao, 1, fpBin);
    reg.nomeEstacao[reg.tamNomeEstacao] = '\0';

    fread(&reg.tamNomeLinha, sizeof(int), 1, fpBin);
    reg.nomeLinha = malloc(reg.tamNomeLinha + 1);
    fread(reg.nomeLinha, reg.tamNomeLinha, 1, fpBin);
    reg.nomeLinha[reg.tamNomeLinha] = '\0';

    // descarta padding
    int restante = TAM_REG_DADOS - sizeof(char) - 9 * sizeof(int)
                   - reg.tamNomeEstacao - reg.tamNomeLinha;
    char lixo[restante];
    fread(lixo, sizeof(char), restante, fpBin);

    return reg;
}

// imprime os campos de um registro formatados em uma linha
void imprimeRegistro(struct registro reg) {
    char buffSaida[TAM_REG_DADOS];
    int pos = 0;

    // monta a linha de saída no buffer, usando formataSeNulo para campos opcionais
    pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%d ", reg.codEstacao);
    pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%s ", reg.nomeEstacao);
    pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%d ", reg.codLinha);
    pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%s ", reg.nomeLinha);
    pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%s ", formataSeNulo(reg.codProxEstacao));
    pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%s ", formataSeNulo(reg.distProxEstacao));
    pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%s ", formataSeNulo(reg.codLinhaIntegra));
    pos += snprintf(buffSaida + pos, sizeof(buffSaida) - pos, "%s",  formataSeNulo(reg.codEstIntegra));

    printf("%s\n", buffSaida);
}

// retorna "NULO" se o valor for -1 (sentinela para campo ausente),
// ou o valor convertido para string caso contrário
char* formataSeNulo(int valor) {
    if (valor == -1)
        return "NULO";
    static char buf[10];
    snprintf(buf, sizeof(buf), "%d", valor);
    return buf;
}
