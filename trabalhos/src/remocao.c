/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arquivo_bin.h"
#include "registro_io.h"
#include "filtro.h"
#include "buscaIndexada.h"
#include "fornecidas.h"
#include "remocao.h"
#include "escrita.h"

int removeRegistros(char *arquivoEntrada, char *arquivoIndice, int n) {
    // abre o arquivo de dados para leitura e escrita sem truncar
    FILE *fpEntrada = fopen(arquivoEntrada, "r+b");
    if (fpEntrada == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return 1;
    }


    // lê e valida o cabeçalho do arquivo de dados
    char status;
    int topo, proxRRN, nroEstacoes, nroParesEstacao;
    if (leCabecalho(fpEntrada, &status, &topo, &proxRRN, &nroEstacoes, &nroParesEstacao) != 0
        || status != '1') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fpEntrada);
        return 1;
    }
    
    // marca como inconsistente
    escreveCabecalho(fpEntrada, '0', topo, proxRRN, nroEstacoes, nroParesEstacao);

    // carrega o índice primário em memória
    int totalIndice;
    ParIndice *pares = carregaIndice(arquivoIndice, &totalIndice);
    if (pares == NULL) {
        printf("Falha no processamento do arquivo.\n");
        fclose(fpEntrada);
        return 1;
    }

    // executa n remoções
    char bufferLinha[1024];
    for (int i = 0; i < n; i++) {
        int m;
        scanf("%d", &m);

        char **nomeCampo  = malloc(m * sizeof(char *));
        char **valorCampo = malloc(m * sizeof(char *));

        // lê os m pares nomeCampo/valorCampo
        int usaIndice = 0;
        int chave = -1;
        for (int j = 0; j < m; j++) {
            scanf("%s", bufferLinha);
            nomeCampo[j] = malloc(strlen(bufferLinha) + 1);
            strcpy(nomeCampo[j], bufferLinha);

            if (strcmp(nomeCampo[j], "nomeEstacao") == 0 ||
                strcmp(nomeCampo[j], "nomeLinha") == 0) {
                ScanQuoteString(bufferLinha);
            } else {
                scanf("%s", bufferLinha);
            }
            valorCampo[j] = malloc(strlen(bufferLinha) + 1);
            strcpy(valorCampo[j], bufferLinha);

            if (strcmp(nomeCampo[j], "codEstacao") == 0) {
                usaIndice = 1;
                chave = atoi(valorCampo[j]);
            }
        }

        // coleta os RRNs dos registros a remover
        int *rrns = NULL;
        int encontrados = 0;
        if (usaIndice) {
            int rrn = buscaBinariaIndice(pares, totalIndice, chave);
            if (rrn != -1) {
                rrns = malloc(sizeof(int));
                rrns[0] = rrn;
                encontrados = 1;
            }
        } else {
            rrns = NULL;
            encontrados = buscaRegistros(fpEntrada, nomeCampo, valorCampo,
                                         m, proxRRN, MODO_COLETAR_RRN, &rrns);
        }

        // remove cada registro encontrado
        for (int j = 0; j < encontrados; j++) {
            int rrn = rrns[j];
            long offset = TAM_REG_CABECALHO + rrn * TAM_REG_DADOS;

            // lê o codEstacao do registro para remover do índice
            struct registro reg = leRegistro(fpEntrada, offset);
            int codEstacaoRemovido = reg.codEstacao;
            free(reg.nomeEstacao);
            free(reg.nomeLinha);

            // escreve marcador de remoção e aponta para o topo atual da pilha
            fseek(fpEntrada, offset, SEEK_SET);
            char marcador = '1';
            fwrite(&marcador, sizeof(char), 1, fpEntrada);
            fwrite(&topo, sizeof(int), 1, fpEntrada);
            fflush(fpEntrada);  // garante que o marcador está no disco antes da próxima leitura

            // atualiza o topo da pilha (cabeçalho será escrito após todas as remoções)
            topo = rrn;

            // remove o par do índice em memória: substitui pelo último e decrementa
            for (int k = 0; k < totalIndice; k++) {
                if (pares[k].codEstacao == codEstacaoRemovido) {
                    pares[k] = pares[totalIndice - 1];
                    totalIndice--;
                    break;
                }
            }
        }

        
        free(rrns);
        fflush(fpEntrada);  // garante que todas as remoções estão no disco antes de ler o próximo comando
        for (int j = 0; j < m; j++) {
            free(nomeCampo[j]);
            free(valorCampo[j]);
        }
        free(nomeCampo);
        free(valorCampo);
        
        
    }

    // recalcula os contadores a partir do estado final do arquivo
    nroEstacoes = 0;
    nroParesEstacao = 0;

    char **nomesVistos = NULL;
    struct parEstacao *paresVistos = NULL;

    for (int rrn = 0; rrn < proxRRN; rrn++) {
        long offset = TAM_REG_CABECALHO + rrn * TAM_REG_DADOS;

        struct registro reg = leRegistro(fpEntrada, offset);

        // ignora registros removidos
        if (reg.removido == '1') {
            free(reg.nomeEstacao);
            free(reg.nomeLinha);
            continue;
        }

        // atualiza nroEstacoes
        if (!nomeEstacaoExiste(nomesVistos, nroEstacoes, &reg)) {
            nomesVistos = realloc(nomesVistos, (nroEstacoes + 1) * sizeof(char *));
            nomesVistos[nroEstacoes] = malloc(reg.tamNomeEstacao + 1);

            memcpy(nomesVistos[nroEstacoes],
                reg.nomeEstacao,
                reg.tamNomeEstacao);

            nomesVistos[nroEstacoes][reg.tamNomeEstacao] = '\0';

            nroEstacoes++;
        }

        // atualiza nroParesEstacao
        struct parEstacao parNovo;
        parNovo.codEstacao1 = reg.codEstacao;
        parNovo.codEstacao2 = reg.codProxEstacao;

        if (reg.codProxEstacao != -1 &&
            !parExiste(paresVistos, nroParesEstacao, parNovo)) {

            paresVistos = realloc(
                paresVistos,
                (nroParesEstacao + 1) * sizeof(struct parEstacao));

            paresVistos[nroParesEstacao] = parNovo;
            nroParesEstacao++;
        }

        free(reg.nomeEstacao);
        free(reg.nomeLinha);
    }
    for (int i = 0; i < nroEstacoes; i++)
        free(nomesVistos[i]);

    free(nomesVistos);
    free(paresVistos);

    // escreve o cabeçalho atualizado uma única vez após todas as remoções
    escreveCabecalho(fpEntrada, '1', topo, proxRRN, nroEstacoes, nroParesEstacao);
    
    free(pares);
    fclose(fpEntrada);
    BinarioNaTela(arquivoEntrada);
    criaIndice(arquivoEntrada, arquivoIndice);
    
    return 0;
}