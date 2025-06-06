// Em main.cc
#include "disk.h"
#include "program.h"
#include "page_table.h"
#include "page_replacement.h" // Inclua seu header
#include <cstring>
#include <iostream> // Para std::cout

int main(int argc, char *argv[])
{
	if(argc != 5) {
		// Mensagem de erro original...
		return 1;
	}

	int npages = atoi(argv[1]);
	int nframes = atoi(argv[2]);
	const char *algorithm = argv[3];
	const char *program = argv[4];

	Disk disk("myvirtualdisk", npages);
    Program my_program;

    Page_Replacement::setup(nframes, algorithm, &disk);

    Page_Table pt(npages, nframes, Page_Replacement::page_fault_handler);
	
	unsigned char *virtmem = (unsigned char *) pt.page_table_get_virtmem();
	
    if(!strcmp(program,"alpha")) {
		my_program.alpha(virtmem, npages * Page_Table::PAGE_SIZE);
	} else if(!strcmp(program,"beta")) {
		my_program.beta(virtmem, npages * Page_Table::PAGE_SIZE);
	} else if(!strcmp(program,"gamma")) {
		my_program.gamma(virtmem, npages * Page_Table::PAGE_SIZE);
	} else if(!strcmp(program,"delta")) {
		my_program.delta(virtmem, npages * Page_Table::PAGE_SIZE);
	}

	// Número de faltas de página
    std::cout << "Page faults: " << Page_Replacement::get_page_faults() << std::endl;
	// Número de leituras no disco
    std::cout << "Disk reads: " << Page_Replacement::get_disk_reads() << std::endl;
	// Número de escritas no disco
    std::cout << "Disk writes: " << Page_Replacement::get_disk_writes() << std::endl;

    pt.page_table_delete();
	disk.close_disk();
	return 0;
}