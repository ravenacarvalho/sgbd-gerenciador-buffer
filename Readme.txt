# Gerenciador de Buffer (SGBD)

Trabalho 1 da disciplina de Sistemas de Gerenciamento de Banco de Dados (2026).
Implementação em C++ de um gerenciador de buffer simples, com quatro políticas de
substituição de página: **FIFO, LRU, MRU e CLOCK**.

**Autores:** `Ravena Carvalho` e `Luis Francisco`
**Professor:** Javam Machado

---

## Visão geral

Cada linha do arquivo de texto (`bancodedados.csv`) representa uma **página**. As páginas
são lidas sob demanda do arquivo e mantidas em um buffer em memória com capacidade
máxima de **5 entradas**.

Cada página do buffer possui:

| Campo | Descrição |
|---|---|
| `page_number` | Identificador da página (chave). `page#1` é a primeira linha após o cabeçalho |
| `content` | Texto da linha |
| `updated` | Variável booleana de atualização, sorteada (TRUE/FALSE) quando a página é carregada |
| `last_access_time` | Tick lógico do último acesso (usado por LRU e MRU) |
| `reference_bit` | Bit de referência (usado pelo CLOCK) |

O gerenciador mantém os contadores **cache hit** (página encontrada no buffer) e
**cache miss** (página precisou ser buscada no arquivo).

## Estrutura do projeto

```
.
├── buffer.h           # struct Page, constantes, variáveis globais e protótipos
├── buffer_core.cpp    # Fetch, DisplayCache, DisplayStats, leitura do arquivo e main
├── evict.cpp          # Evict e as quatro políticas de substituição
├── bancodedados.csv   # arquivo de entrada (cabeçalho "text" + uma página por linha)
├── Makefile
└── README.md
```

## Compilação

Requer `g++` com suporte a C++11.

```bash
make        # gera o executável ./buffer_mgr
make clean  # remove o executável
```

## Execução

```bash
./buffer_mgr [arquivo.csv]
```

Se nenhum arquivo for informado, usa `bancodedados.csv`.

O programa é interativo:

1. Exibe um menu para escolher a política de substituição:
   `0` FIFO, `1` LRU, `2` MRU, `3` CLOCK (opção inválida assume LRU).
2. Pede repetidamente o número de uma página para `Fetch`. Após cada operação o
   estado do buffer é exibido.
3. Digitar `0` encerra o loop e exibe as estatísticas finais.

### Exemplo de sessão

```
=== GERENCIADOR DE BUFFER ===
Opcao: 1
Politica selecionada: LRU

Digite o numero da pagina para Fetch (ou 0 para encerrar): 3
[MISS] page#3 nao esta no buffer, buscando no arquivo...
[LOAD] page#3 carregada no slot 0 | updated=TRUE
...
--- estatisticas ---
cache hit : 4
cache miss: 7
```

## Interface do gerenciador

| Função | Descrição |
|---|---|
| `Fetch(key, arquivo, politica)` | Consulta o buffer; em caso de **hit** retorna o conteúdo e atualiza os controles de acesso. Em caso de **miss** lê a página do arquivo, chama `Evict()` se o buffer estiver cheio, sorteia o valor de `updated` e insere a página |
| `Evict(politica, buffer, clock_pointer)` | Escolhe a vítima conforme a política, exibe o conteúdo da página removida e acrescenta `W` se `updated == TRUE` |
| `DisplayCache()` | Exibe as entradas do buffer (Chave → nº da página, Valor → texto, Atualização → booleano) |
| `DisplayStats()` | Exibe cache hit, cache miss, total e taxa de acerto |

## Políticas de substituição

| Política | Critério de remoção |
|---|---|
| **FIFO** | Remove a página mais antiga no buffer (primeira posição). Hits não alteram a ordem |
| **LRU** | Remove a página com o menor `last_access_time` (usada há mais tempo) |
| **MRU** | Remove a página com o maior `last_access_time` (usada mais recentemente) |
| **CLOCK** | Percorre o buffer circularmente a partir de `clock_pointer`: se `reference_bit` é `true`, zera e avança; se é `false`, remove essa página |

## Decisões de implementação

- **Tick lógico:** `tempo_logico` é incrementado a cada `Fetch` e usado no lugar de
  timestamps do sistema, garantindo ordenação exata dos acessos.
- **Randomização:** `updated` é sorteado com `rand()`, semeado por `time(nullptr)`,
  então muda a cada execução.
- **Página inexistente:** se o número pedido for maior que o total de linhas do arquivo,
  `Fetch` emite um aviso e retorna `nullptr` (conta como miss, sem remover nada do buffer).
- **Aspas no CSV:** aspas duplas que envolvem a linha inteira são removidas na leitura.

## Arquivo de entrada

CSV com cabeçalho `text` na primeira linha. Cada linha seguinte é uma página:
`page#1` = linha 2 do arquivo, `page#2` = linha 3, e assim por diante.