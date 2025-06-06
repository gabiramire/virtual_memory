// page_replacement.cc

#include "page_replacement.h"
#include <cstdlib> // Para rand() e srand()
#include <ctime>   // Para time()
#include <iostream>

// --- Inicialização dos Membros Estáticos ---
std::string Page_Replacement::algorithm_name;
Disk* Page_Replacement::disk_ptr = nullptr;
int Page_Replacement::nframes = 0;

std::vector<Page_Replacement::Frame> Page_Replacement::frame_table;
std::queue<int> Page_Replacement::fifo_queue;
std::vector<int> Page_Replacement::lru_counters;

int Page_Replacement::page_fault_count = 0;
int Page_Replacement::disk_read_count = 0;
int Page_Replacement::disk_write_count = 0;

// --- Implementação dos Métodos Públicos ---

void Page_Replacement::setup(int num_frames, const std::string& algo_name, Disk* d_ptr) {
    algorithm_name = algo_name;
    disk_ptr = d_ptr;
    nframes = num_frames;

    // Prepara as estruturas de dados com o tamanho correto
    frame_table.resize(nframes);
    lru_counters.resize(nframes, 0);

    // Inicializa o gerador de números aleatórios
    srand(time(NULL));
}

int Page_Replacement::get_page_faults() { return page_fault_count; }
int Page_Replacement::get_disk_reads() { return disk_read_count; }
int Page_Replacement::get_disk_writes() { return disk_write_count; }

void Page_Replacement::page_fault_handler(Page_Table *pt, int page) {
    page_fault_count++;

    // Primeiro, verifique se a página já está na memória (falta de proteção).
    int frame, bits;
    pt->page_table_get_entry(page, &frame, &bits);
    if (bits & PROT_READ) {
        // A página já está na memória, a falta deve ser por permissão de escrita.
        pt->page_table_set_entry(page, frame, PROT_READ | PROT_WRITE);
        if (algorithm_name == "custom") {
            update_lru_counters(frame);
        }
        return; // Falta resolvida.
    }

    // A página não está na memória. Precisamos carregá-la.
    char* physmem_ptr = (char*) pt->page_table_get_physmem();
    int target_frame = find_free_frame();

    if (target_frame == -1) { // Memória cheia, precisamos substituir uma página.
        // Escolha a vítima de acordo com o algoritmo
        if (algorithm_name == "rand") {
            target_frame = random_replacement();
        } else if (algorithm_name == "fifo") {
            target_frame = fifo_replacement();
        } else if (algorithm_name == "custom") {
            target_frame = custom_lru_replacement();
        } else {
            std::cerr << "Algoritmo desconhecido: " << algorithm_name << std::endl;
            exit(1);
        }
        
        // Lida com a página vítima (escreve no disco se necessário, etc.)
        handle_victim(target_frame, pt, physmem_ptr);
    }
    
    // Agora temos um 'target_frame' (seja ele um frame novo ou de uma vítima).
    // Carregue a página requisitada do disco para este frame.
    disk_ptr->read(page, physmem_ptr + (target_frame * Page_Table::PAGE_SIZE));
    disk_read_count++;

    // Atualize a tabela de páginas para a nova página.
    pt->page_table_set_entry(page, target_frame, PROT_READ);

    // Atualize nossas estruturas de dados internas.
    frame_table[target_frame].is_occupied = true;
    frame_table[target_frame].page_number = page;
    
    if (algorithm_name == "fifo") {
        fifo_queue.push(target_frame);
    } else if (algorithm_name == "custom") {
        update_lru_counters(target_frame);
    }
}


// --- Implementação dos Métodos Privados ---

int Page_Replacement::find_free_frame() {
    for (int i = 0; i < nframes; ++i) {
        if (!frame_table[i].is_occupied) {
            return i;
        }
    }
    return -1; // Nenhum frame livre
}

void Page_Replacement::handle_victim(int frame_index, Page_Table *pt, char *physmem) {
    int victim_page = frame_table[frame_index].page_number;

    // Verifique se a página vítima foi modificada ("dirty")
    int victim_frame_temp, victim_bits;
    pt->page_table_get_entry(victim_page, &victim_frame_temp, &victim_bits);

    if (victim_bits & PROT_WRITE) {
        // Se sim, escreva-a de volta para o disco.
        disk_ptr->write(victim_page, physmem + (frame_index * Page_Table::PAGE_SIZE));
        disk_write_count++;
    }

    // Invalide a entrada da tabela de páginas da vítima.
    pt->page_table_set_entry(victim_page, 0, 0);
}

int Page_Replacement::random_replacement() {
    return rand() % nframes;
}

int Page_Replacement::fifo_replacement() {
    int frame_to_replace = fifo_queue.front();
    fifo_queue.pop();
    return frame_to_replace;
}

// Algoritmo Custom: LRU (Least Recently Used)
// A página com o maior contador de "idade" é a menos recentemente usada.
int Page_Replacement::custom_lru_replacement() {
    int max_age = -1;
    int victim_frame = -1;
    for (int i = 0; i < nframes; ++i) {
        if (lru_counters[i] > max_age) {
            max_age = lru_counters[i];
            victim_frame = i;
        }
    }
    return victim_frame;
}

// Para LRU: incrementa a "idade" de todos, e zera a do frame recém-usado.
void Page_Replacement::update_lru_counters(int most_recently_used_frame) {
    for (int i = 0; i < nframes; ++i) {
        lru_counters[i]++;
    }
    lru_counters[most_recently_used_frame] = 0;
}