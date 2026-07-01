/*
Fernando Barbosa Leite - 14570458
Luís Filipe Vasconcelos Peres - 10310641
*/

#ifndef HEAP_H
#define HEAP_H

// nó do heap — representa um vértice com sua distância acumulada
// nome é apenas uma referência (não é alocado nem liberado pelo heap);
// deve apontar para uma string que permaneça válida enquanto o heap existir
// (por exemplo, o nomeEstacao já alocado dentro do Grafo)
typedef struct {
    int idx;      // índice do vértice no vetor de vértices do Grafo
    int dist;     // distância acumulada até este vértice
    char *nome;   // nome da estação, usado para desempate
} HeapNode;

// min-heap baseado em vetor, com desempate por nome da estação
typedef struct {
    HeapNode *nos;  // vetor de nós do heap
    int tamanho;    // quantidade de elementos atualmente no heap
    int capacidade; // capacidade atual do vetor (para realloc)
} MinHeap;

// aloca e inicializa um heap vazio
MinHeap *criaHeap();

// insere um novo nó no heap, mantendo a propriedade de min-heap
// desempate: menor dist primeiro; em caso de empate, menor nome (strcmp)
void insereHeap(MinHeap *h, int idx, int dist, char *nome);

// remove e retorna o nó de menor prioridade (menor dist, desempatando por nome)
// comportamento indefinido se o heap estiver vazio — verificar com heapVazio antes
HeapNode extraiMin(MinHeap *h);

// retorna 1 se o heap estiver vazio, 0 caso contrário
int heapVazio(MinHeap *h);

// libera a memória alocada pelo heap (não libera os nomes referenciados)
void liberaHeap(MinHeap *h);

#endif // HEAP_H
