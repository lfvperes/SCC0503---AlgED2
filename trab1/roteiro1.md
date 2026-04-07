# Roteiro de Apresentação — Arquivo Binário de Estações

---

## 1. Programa Principal (`programaTrab.c`)
- Lê até 3 tokens do `stdin` com `scanf` num loop (sem `argc/argv` da linha de comando)
- `argv[0]` é a funcionalidade (1–4), convertida com `atoi`
- `switch` despacha para a função correspondente, passando nome(s) de arquivo e parâmetros extras
- **Ponto a destacar:** a interface é simples de propósito — toda a lógica fica encapsulada nas funções

---

## 2. `criaTabela` — Função 1

**Structs definidos em `registro.h`:**
- `struct registro` — campos fixos (`int`, `char`) + campos de tamanho variável (`tamNome` + ponteiro), refletindo o layout do registro binário de 80 bytes
- `struct parEstacao` — par de códigos para rastrear conexões únicas entre estações
- `enum CampoRegistro` — enumera todos os campos, usado pelas funções de leitura para evitar strings espalhadas

**Funções auxiliares (`static`, só visíveis no arquivo):**
- `lerRegistroCSV` — usa `strsep` para parsear a linha do CSV campo a campo; valores vazios viram `-1` (sentinela de nulo); aloca memória para os campos string
- `escreveRegistroCabecalho` — grava o cabeçalho de 17 bytes com `memcpy`; escreve status `'0'` (inconsistente) no início e atualiza após cada registro → garante consistência mesmo em caso de falha
- `escreveRegistroDados` — serializa a `struct` num buffer fixo de 80 bytes com `memcpy`; campos variáveis são precedidos pelo tamanho; bytes restantes são preenchidos com `'$'`
- `nomeEstacaoExiste` — percorre registros já gravados comparando `tamNomeEstacao` + conteúdo; evita duplicatas no contador de estações
- `parExiste` / `normalizaPar` / `paresSaoEquivalentes` — normaliza a ordem do par (menor código primeiro) antes de comparar, garantindo que A→B e B→A sejam o mesmo par

**Fluxo da `criaTabela`:**
1. Abre CSV e binário
2. Grava cabeçalho inicial com status `'0'`
3. Descarta cabeçalho do CSV (`fgetc` até `'\n'`)
4. Para cada linha: parseia → verifica duplicatas → serializa → atualiza cabeçalho
5. Ao final, regrava cabeçalho com status `'1'` (consistente)
6. Chama `BinarioNaTela` para imprimir o resultado

---

## 3. `imprimeTabela` — Função 2

**Fluxo:**
1. Abre o binário; lê os 17 bytes do cabeçalho
2. Verifica `status == '0'` → erro de consistência
3. Itera de `i = 0` até `proxRRN - 1`; para cada registro:
   - Chama `leRegistro(fpBin, offset)` que faz `fseek` + lê campo a campo (incluindo `malloc` dos strings variáveis)
   - Pula se `removido == '1'`
   - Chama `imprimeRegistro` que monta a saída num buffer local com `snprintf` e depois dá um único `printf`
4. `formataSeNulo` centraliza a lógica de imprimir `"NULO"` para campos com valor `-1`

**Funções auxiliares reutilizadas também pelas funções 3 e 4:**
- `leRegistro` — lê qualquer registro dado um offset absoluto; descarta bytes de padding no final
- `imprimeRegistro` — formata e imprime uma linha por registro
- `formataSeNulo` — converte `-1` → `"NULO"` para campos opcionais

---

## 4. `listaTabelaFiltro` — Função 3

**Fluxo:**
1. Lê cabeçalho → verifica consistência
2. Lê `n` conjuntos de filtros do `stdin`; para cada um:
   - Lê `m` (número de filtros)
   - Para cada filtro: lê nome do campo (`scanf`) e valor — usa `ScanQuoteString` para `nomeEstacao`/`nomeLinha` (trata aspas e espaços)
   - Chama `buscaRegistros`

**`buscaRegistros` — dois passes:**
- **Passe 1:** percorre todos os registros, ignora removidos, aplica todos os `m` filtros em AND; conta os que passam
- Aloca `malloc(encontrados * sizeof(struct registro))`
- **Passe 2:** repete a varredura e preenche o array com `leRegistro` para cada correspondência

**`verificaCampo`:** recebe o campo como `CampoRegistro` (via `nomeCampoParaEnum`), faz `fseek` para o offset correto e compara o valor lido; para `nomeLinha` é preciso ler `tamNomeEstacao` primeiro para calcular onde `nomeLinha` começa — campos de tamanho variável tornam o offset dependente do conteúdo

---

## 5. `acessoRRN` — Função 4

**Fluxo:**
1. Lê cabeçalho → verifica consistência
2. Calcula offset diretamente: `TAM_REG_CABECALHO + RRN × TAM_REG_DADOS`
   - **Ponto central:** como todos os registros têm tamanho fixo de 80 bytes, o acesso é **O(1)** — não há varredura
3. Chama `leRegistro(fpBin, offset)` → verifica `removido == '1'` → `imprimeRegistro`
4. Reutiliza exatamente os mesmos auxiliares das funções 2 e 3

---

## Considerações Finais (opcional, se der tempo)
- Separação de responsabilidades: escrita em `criaTabela.c`, leitura em `leitura.c`, definições compartilhadas em `registro.h`
- Funções `static` evitam poluição de namespace
- Gerenciamento de memória explícito: todo `malloc` tem um `free` correspondente
- Status de consistência no cabeçalho protege contra leituras de arquivos corrompidos