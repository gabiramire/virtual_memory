// page_replacement.h

#ifndef PAGE_REPLACEMENT_H
#define PAGE_REPLACEMENT_H

#include "page_table.h"
#include "disk.h"
#include <string>
#include <vector>
#include <queue>

class Page_Replacement
{
public:
    // --- MÉTODOS PÚBLICOS ---

    /**
     * @brief Configura o sistema de substituição de páginas antes de iniciar a simulação.
     * @param num_frames O número de frames na memória física.
     * @param algo_name O nome do algoritmo a ser usado ("rand", "fifo", "custom").
     * @param d_ptr Um ponteiro para o objeto de disco.
     */
    static void setup(int num_frames, const std::string& algo_name, Disk* d_ptr);

    /**
     * @brief O tratador de falta de páginas. Este é o coração do projeto.
     * Chamado automaticamente pelo Page_Table em uma falha de segmentação.
     */
    static void page_fault_handler(Page_Table *pt, int page);

    /**
     * @brief Retorna o número total de faltas de página contadas.
     */
    static int get_page_faults();

    /**
     * @brief Retorna o número total de leituras do disco contadas.
     */
    static int get_disk_reads();

    /**
     * @brief Retorna o número total de escritas no disco contadas.
     */
    static int get_disk_writes();


private:
    // --- ESTRUTURAS E MEMBROS DE DADOS ESTÁTICOS ---

    // Estrutura interna para representar o estado de um frame físico.
    struct Frame {
        bool is_occupied = false;
        int page_number = -1;
    };

    // Variáveis de configuração
    static std::string algorithm_name;
    static Disk* disk_ptr;
    static int nframes;

    // Estruturas de dados para gerenciamento de frames e algoritmos
    static std::vector<Frame> frame_table;      // Nossa tabela de frames
    static std::queue<int> fifo_queue;          // Fila para o algoritmo FIFO
    static std::vector<int> lru_counters;       // Contadores para nosso algoritmo Custom (LRU)

    // Contadores de estatísticas
    static int page_fault_count;
    static int disk_read_count;
    static int disk_write_count;

    // --- MÉTODOS AUXILIARES PRIVADOS ---

    // Encontra o primeiro frame livre na nossa tabela de frames. Retorna -1 se não houver.
    static int find_free_frame();

    // Lida com a remoção de uma página vítima de um frame.
    static void handle_victim(int frame_index, Page_Table *pt, char *physmem);
    
    // Algoritmos de substituição
    static int random_replacement();
    static int fifo_replacement();
    static int custom_lru_replacement();

    // Atualiza os contadores para o algoritmo LRU.
    static void update_lru_counters(int most_recently_used_frame);
};

#endif