// nucleo do gerenciador de buffer
// contem as funcoes Fetch, DisplayCache e DisplayStats, alem da leitura do arquivo CSV e controle de cache hit/miss

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "buffer.h"

// estado global do buffer
std::vector<Page> buffer;
int clock_pointer = 0; // ponteiro circular usado pela politica CLOCK
int tempo_logico  = 0; // tick logico incrementado a cada Fetch

int cache_hit  = 0;
int cache_miss = 0;

// procura uma pagina no buffer pelo page_number
// retorna o indice do slot se encontrar, -1 caso contrario
static int buscar_no_buffer(int page_number) {
    for (int i = 0; i < (int)buffer.size(); i++) {
        if (buffer[i].page_number == page_number)
            return i;
    }
    return -1;
}

// le uma linha do CSV ignorando o cabecalho
// retorna true se a pagina existe, false se o numero for maior que o total de linhas
static bool ler_do_arquivo(const std::string& caminho, int page_number,
                            std::string& resultado) {
    std::ifstream arq(caminho);
    if (!arq.is_open()) {
        std::cerr << "[ERROR] Não foi possível abrir o arquivo: " << caminho << "\n";
        return false;
    }

    std::string linha;

    // descarta o cabecalho "text"
    if (!std::getline(arq, linha))
        return false;

    int pagina_atual = 0;
    while (std::getline(arq, linha)) {
        pagina_atual++;
        if (pagina_atual == page_number) {
            // algumas linhas do CSV vem entre aspas duplas, remove se for o caso
            if (linha.size() >= 2 && linha.front() == '"' && linha.back() == '"')
                linha = linha.substr(1, linha.size() - 2);
            resultado = linha;
            return true;
        }
    }

    return false;
}

// busca a pagina 'key' no buffer; se nao estiver, carrega do arquivo
// chama Evict antes de inserir caso o buffer esteja cheio
// retorna ponteiro para a entrada no buffer, ou nullptr se a pagina nao existir
Page* Fetch(int key, const std::string& caminho_arquivo, int politica) {
    tempo_logico++;

    int idx = buscar_no_buffer(key);

    if (idx != -1) {
        // pagina ja esta no buffer
        cache_hit++;
        buffer[idx].last_access_time = tempo_logico;
        buffer[idx].reference_bit    = true;
        std::cout << "[HIT]  page#" << key << " encontrada no slot " << idx << "\n";
        return &buffer[idx];
    }

    // pagina nao esta no buffer, vai buscar no arquivo
    cache_miss++;
    std::cout << "[MISS] page#" << key << " nao esta no buffer, buscando no arquivo...\n";

    std::string conteudo;
    if (!ler_do_arquivo(caminho_arquivo, key, conteudo)) {
        std::cerr << "[WARN] page#" << key << " nao existe no arquivo.\n";
        return nullptr;
    }

    // buffer cheio: remove uma pagina antes de inserir
    if ((int)buffer.size() == BUFFER_CAPACITY)
        Evict(politica, buffer, clock_pointer);

    Page nova;
    nova.page_number      = key;
    nova.content          = conteudo;
    nova.updated          = (rand() % 2 == 1); // TRUE ou FALSE sorteado conforme especificacao
    nova.last_access_time = tempo_logico;
    nova.reference_bit    = true;

    buffer.push_back(nova);

    // captura o indice apos o push_back para evitar ponteiro invalido por realloc
    int slot_novo = (int)buffer.size() - 1;

    std::cout << "[LOAD] page#" << key << " carregada no slot " << slot_novo
              << " | updated=" << (nova.updated ? "TRUE" : "FALSE") << "\n";

    return &buffer[slot_novo];
}

// exibe todas as entradas presentes no buffer
void DisplayCache() {
    std::cout << "\n--- estado do buffer ---\n";

    if (buffer.empty()) {
        std::cout << "(buffer vazio)\n\n";
        return;
    }

    for (int i = 0; i < (int)buffer.size(); i++) {
        const Page& p = buffer[i];
        std::cout << "slot " << i << "\n";
        std::cout << "  Chave      -> " << p.page_number << "\n";
        std::cout << "  Valor      -> " << p.content << "\n";
        std::cout << "  Atualizacao-> " << (p.updated ? "TRUE" : "FALSE") << "\n";
    }

    std::cout << "ocupados: " << buffer.size() << "/" << BUFFER_CAPACITY << "\n\n";
}

// exibe cache hit, cache miss e taxa de acerto
void DisplayStats() {
    int total   = cache_hit + cache_miss;
    double taxa = (total > 0) ? (100.0 * cache_hit / total) : 0.0;

    std::cout << "\n--- estatisticas ---\n";
    std::cout << "cache hit : " << cache_hit  << "\n";
    std::cout << "cache miss: " << cache_miss << "\n";
    std::cout << "total     : " << total      << "\n";
    std::cout << "acerto    : " << taxa       << "%\n\n";
}

int main(int argc, char* argv[]) {
    srand((unsigned int)time(nullptr));
    std::string arquivo = "bancodedados.csv";

    // Se passar o nome do arquivo por linha de comando, ele pega
    if (argc >= 2) arquivo = argv[1];

    // --- MENU INTERATIVO PARA ESCOLHER A POLÍTICA ---
    int opcao_politica;
    std::cout << "=== GERENCIADOR DE BUFFER ===\n";
    std::cout << "Escolha a politica de substituicao:\n";
    std::cout << "0 - FIFO\n";
    std::cout << "1 - LRU\n";
    std::cout << "2 - MRU\n";
    std::cout << "3 - CLOCK\n";
    std::cout << "Opcao: ";
    std::cin >> opcao_politica;

    int politica;
    switch (opcao_politica) {
        case 0: politica = POLITICA_FIFO; std::cout << "\nPolitica selecionada: FIFO\n"; break;
        case 1: politica = POLITICA_LRU;  std::cout << "\nPolitica selecionada: LRU\n"; break;
        case 2: politica = POLITICA_MRU;  std::cout << "\nPolitica selecionada: MRU\n"; break;
        case 3: politica = POLITICA_CLOCK; std::cout << "\nPolitica selecionada: CLOCK\n"; break;
        default: 
            std::cerr << "Opcao invalida! Padrao LRU selecionado.\n"; 
            politica = POLITICA_LRU; 
            break;
    }
    std::cout << "Arquivo base: " << arquivo << "\n\n";

    // --- LOOP DE FETCH SOB DEMANDA ---
    int num_pagina;
    while (true) {
        std::cout << "------------------------------------------------\n";
        std::cout << "Digite o numero da pagina para Fetch (ou 0 para encerrar): ";
        std::cin >> num_pagina;

        // Se o usuário digitar 0, quebra o loop e vai para as estatísticas
        if (num_pagina == 0) {
            break; 
        }

        std::cout << "\n-- Fetch(" << num_pagina << ") --\n";
        Page* p = Fetch(num_pagina, arquivo, politica);
        
        if (p) {
            std::cout << "conteudo: \"" << p->content << "\"\n";
        }
        
        // Exibe o buffer atualizado após cada operação
        DisplayCache(); 
    }

    // Quando sair do loop (digitou 0), exibe o painel final
    std::cout << "\nEncerrando o sistema...\n";
    DisplayStats();
    
    return 0;
}