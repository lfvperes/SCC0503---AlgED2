/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef GRAFO_H
#define GRAFO_H

// nó da lista de adjacências — representa uma aresta direcionada saindo de um vértice
typedef struct ArestaAdj {
    char *nomeProxEstacao;  // nome do vértice destino
    int distancia;          // peso da aresta (0 para integrações)
    char **linhas;          // array de nomes de linha (pode haver mais de uma)
    int nLinhas;            // quantidade de linhas armazenadas
    struct ArestaAdj *prox; // próximo nó da lista encadeada
} ArestaAdj;

// vértice do grafo — representa uma estação única (pelo nome)
typedef struct {
    char *nomeEstacao;  // nome da estação (chave do vértice)
    ArestaAdj *lista;   // lista encadeada de arestas adjacentes, ordenada por nomeProxEstacao
} Vertice;

// grafo direcionado ponderado representado por lista de adjacências
typedef struct {
    Vertice *vertices;  // vetor de vértices, ordenado crescentemente por nomeEstacao
    int nVertices;      // número de vértices
    int capacidade;     // capacidade atual do vetor (para realloc)
} Grafo;

// aloca e inicializa um grafo vazio
Grafo *criaGrafo();

// busca um vértice pelo nome; retorna seu índice no vetor ou -1 se não encontrado
int buscaVertice(Grafo *g, char *nome);

// insere um novo vértice com o nome dado, mantendo o vetor ordenado crescentemente
void insereVertice(Grafo *g, char *nome);

// insere uma aresta (nomeU → nomeV) com a distância e nome de linha dados;
// se já existir aresta para nomeV a partir de nomeU, apenas adiciona nomeLinha
void insereAresta(Grafo *g, char *nomeU, char *nomeV, int dist, char *nomeLinha);

// imprime o grafo no formato especificado pela funcionalidade 10
void imprimeGrafo(Grafo *g);

// libera toda a memória alocada pelo grafo
void liberaGrafo(Grafo *g);

#endif // GRAFO_H