/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arquivo_bin.h"
#include "registro_io.h"
#include "fornecidas.h"
#include "criaIndice.h"
#include "buscaIndexada.h"
#include "escrita.h"
#include "filtro.h"
#include "atualizaRegistro.h"
#include "insereRegistro.h"

int atualizaRegistros(char *arquivoEntrada, char *arquivoIndice, int n) {
    // --- abre e valida o arquivo de índice ---
    FILE *fpIndice = fopen(arquivoIndice, "rb");
    if (fpIndice == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return 1;
    }

    char statusIndice;
    if (fread(&statusIndice, sizeof(char), 1, fpIndice) != 1 || statusIndice != '1') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fpIndice);
        return 1;
    }

    ParIndice *pares = NULL;
    int total = carregaPares(fpIndice, &pares);
    fclose(fpIndice);

    if (total < 0) {
        printf("Falha no processamento do arquivo.\n");
        return 1;
    }

    // --- abre e valida o arquivo de dados ---
    FILE *fpDados = fopen(arquivoEntrada, "r+b");
    if (fpDados == NULL) {
        printf("Falha no processamento do arquivo.\n");
        free(pares);
        return 1;
    }

    char status;
    int topo, proxRRN, nroEstacoes, nroParesEstacao;
    if (leCabecalho(fpDados, &status, &topo, &proxRRN, &nroEstacoes, &nroParesEstacao) != 0
        || status != '1') {
        printf("Falha no processamento do arquivo.\n");
        free(pares);
        fclose(fpDados);
        return 1;
    }

    // marca como inconsistente uma única vez antes do loop
    escreveCabecalho(fpDados, '0', topo, proxRRN, nroEstacoes, nroParesEstacao);

    for (int i = 0; i < n; i++) {
        // --- lê m pares de busca ---
        int m;
        scanf("%d", &m);

        char **nomeBusca  = malloc(m * sizeof(char *));
        char **valorBusca = malloc(m * sizeof(char *));

        int usaIndice = 0;
        int chave = -1;
        char bufferLinha[1024];

        for (int j = 0; j < m; j++) {
            scanf("%s", bufferLinha);
            nomeBusca[j] = malloc(strlen(bufferLinha) + 1);
            strcpy(nomeBusca[j], bufferLinha);

            if (strcmp(nomeBusca[j], "nomeEstacao") == 0 ||
                strcmp(nomeBusca[j], "nomeLinha") == 0) {
                ScanQuoteString(bufferLinha);
            } else {
                scanf("%s", bufferLinha);
            }
            valorBusca[j] = malloc(strlen(bufferLinha) + 1);
            strcpy(valorBusca[j], bufferLinha);

            if (strcmp(nomeBusca[j], "codEstacao") == 0) {
                usaIndice = 1;
                chave = atoi(valorBusca[j]);
            }
        }

        // --- lê p pares de atualização ---
        int p;
        scanf("%d", &p);

        char **nomeAtu  = malloc(p * sizeof(char *));
        char **valorAtu = malloc(p * sizeof(char *));

        for (int j = 0; j < p; j++) {
            scanf("%s", bufferLinha);
            nomeAtu[j] = malloc(strlen(bufferLinha) + 1);
            strcpy(nomeAtu[j], bufferLinha);

            if (strcmp(nomeAtu[j], "nomeEstacao") == 0 ||
                strcmp(nomeAtu[j], "nomeLinha") == 0) {
                ScanQuoteString(bufferLinha);
            } else {
                scanf("%s", bufferLinha);
            }
            valorAtu[j] = malloc(strlen(bufferLinha) + 1);
            strcpy(valorAtu[j], bufferLinha);
        }

        // --- busca os RRNs a atualizar (indexada ou sequencial) ---
        int *rrns = NULL;
        int encontrados = 0;

        if (usaIndice) {
            int rrn = buscaBinariaIndice(pares, total, chave);
            if (rrn != -1) {
                rrns = malloc(sizeof(int));
                rrns[0] = rrn;
                encontrados = 1;
            } else {
                encontrados = buscaRegistros(fpDados, nomeBusca, valorBusca,
                                             m, proxRRN, MODO_COLETAR_RRN, &rrns);
            }
        } else {
            encontrados = buscaRegistros(fpDados, nomeBusca, valorBusca,
                                         m, proxRRN, MODO_COLETAR_RRN, &rrns);
        }

        // --- para cada RRN encontrado: lê, modifica, reescreve e atualiza contagens ---
        for (int j = 0; j < encontrados; j++) {
            int rrn = rrns[j];
            long offset = TAM_REG_CABECALHO + rrn * (long)TAM_REG_DADOS;

            // lê o registro atual do disco
            struct registro reg = leRegistro(fpDados, offset);

            // guarda valores antigos para atualizar contagens depois
            int codProxAntigo   = reg.codProxEstacao;
            int tamNomeAntigo   = reg.tamNomeEstacao;
            char *nomeAntigo    = malloc(tamNomeAntigo + 1);
            memcpy(nomeAntigo, reg.nomeEstacao, tamNomeAntigo + 1);

            // aplica cada par de atualização
            for (int k = 0; k < p; k++) {
                if (strcmp(nomeAtu[k], "codEstacao") == 0) {
                    reg.codEstacao = atoi(valorAtu[k]);
                } else if (strcmp(nomeAtu[k], "nomeEstacao") == 0) {
                    free(reg.nomeEstacao);
                    reg.tamNomeEstacao = strlen(valorAtu[k]);
                    reg.nomeEstacao = malloc(reg.tamNomeEstacao + 1);
                    memcpy(reg.nomeEstacao, valorAtu[k], reg.tamNomeEstacao + 1);
                } else if (strcmp(nomeAtu[k], "codLinha") == 0) {
                    reg.codLinha = atoi(valorAtu[k]);
                } else if (strcmp(nomeAtu[k], "nomeLinha") == 0) {
                    free(reg.nomeLinha);
                    reg.tamNomeLinha = strlen(valorAtu[k]);
                    reg.nomeLinha = malloc(reg.tamNomeLinha + 1);
                    memcpy(reg.nomeLinha, valorAtu[k], reg.tamNomeLinha + 1);
                } else if (strcmp(nomeAtu[k], "codProxEstacao") == 0) {
                    reg.codProxEstacao = (strcmp(valorAtu[k], "NULO") == 0) ? -1 : atoi(valorAtu[k]);
                } else if (strcmp(nomeAtu[k], "distProxEstacao") == 0) {
                    reg.distProxEstacao = (strcmp(valorAtu[k], "NULO") == 0) ? -1 : atoi(valorAtu[k]);
                } else if (strcmp(nomeAtu[k], "codLinhaIntegra") == 0) {
                    reg.codLinhaIntegra = (strcmp(valorAtu[k], "NULO") == 0) ? -1 : atoi(valorAtu[k]);
                } else if (strcmp(nomeAtu[k], "codEstIntegra") == 0) {
                    reg.codEstIntegra = (strcmp(valorAtu[k], "NULO") == 0) ? -1 : atoi(valorAtu[k]);
                }
            }

            // reescreve o registro modificado no mesmo offset
            fseek(fpDados, offset, SEEK_SET);
            escreveRegistroDados(fpDados, &reg);

            // atualiza índice em memória se codEstacao foi modificado
            for (int k = 0; k < total; k++) {
                if (pares[k].rrn == rrn) {
                    pares[k].codEstacao = reg.codEstacao;
                    break;
                }
            }

            // atualiza nroParesEstacao se codProxEstacao mudou
            if (codProxAntigo != reg.codProxEstacao) {
                if (codProxAntigo != -1)
                    nroParesEstacao--;
                if (reg.codProxEstacao != -1)
                    nroParesEstacao++;
            }

            // atualiza nroEstacoes se nomeEstacao mudou
            int nomeIgual = (reg.tamNomeEstacao == tamNomeAntigo &&
                             memcmp(reg.nomeEstacao, nomeAntigo, tamNomeAntigo) == 0);
            if (!nomeIgual) {
                // checa se o nome antigo ainda existe em outro registro ativo
                int nomeAntigoAindaExiste = 0;
                for (int k = 0; k < proxRRN; k++) {
                    if (k == rrn) continue;
                    long offK = TAM_REG_CABECALHO + k * (long)TAM_REG_DADOS;
                    struct registro regK = leRegistro(fpDados, offK);
                    if (regK.removido == '0' &&
                        regK.tamNomeEstacao == tamNomeAntigo &&
                        memcmp(regK.nomeEstacao, nomeAntigo, tamNomeAntigo) == 0) {
                        nomeAntigoAindaExiste = 1;
                        free(regK.nomeEstacao);
                        free(regK.nomeLinha);
                        break;
                    }
                    free(regK.nomeEstacao);
                    free(regK.nomeLinha);
                }
                if (!nomeAntigoAindaExiste)
                    nroEstacoes--;

                // checa se o nome novo já existe em outro registro ativo
                int nomeNovoJaExiste = 0;
                for (int k = 0; k < proxRRN; k++) {
                    if (k == rrn) continue;
                    long offK = TAM_REG_CABECALHO + k * (long)TAM_REG_DADOS;
                    struct registro regK = leRegistro(fpDados, offK);
                    if (regK.removido == '0' &&
                        regK.tamNomeEstacao == reg.tamNomeEstacao &&
                        memcmp(regK.nomeEstacao, reg.nomeEstacao, reg.tamNomeEstacao) == 0) {
                        nomeNovoJaExiste = 1;
                        free(regK.nomeEstacao);
                        free(regK.nomeLinha);
                        break;
                    }
                    free(regK.nomeEstacao);
                    free(regK.nomeLinha);
                }
                if (!nomeNovoJaExiste)
                    nroEstacoes++;
            }

            free(nomeAntigo);
            free(reg.nomeEstacao);
            free(reg.nomeLinha);
        }

        free(rrns);

        // libera pares de busca e atualização
        for (int j = 0; j < m; j++) {
            free(nomeBusca[j]);
            free(valorBusca[j]);
        }
        free(nomeBusca);
        free(valorBusca);

        for (int j = 0; j < p; j++) {
            free(nomeAtu[j]);
            free(valorAtu[j]);
        }
        free(nomeAtu);
        free(valorAtu);
    }

    // escreve cabeçalho final consistente
    escreveCabecalho(fpDados, '1', topo, proxRRN, nroEstacoes, nroParesEstacao);
    fclose(fpDados);
    free(pares);

    BinarioNaTela(arquivoEntrada);
    criaIndice(arquivoEntrada, arquivoIndice);
    return 0;
}