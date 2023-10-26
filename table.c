#include <stdlib.h>
#include "table.h"
#include "user.h"

Table *init_table() {
	Table *table = malloc(sizeof(Table));
	table->n_records = 0;

	return table;
}


void load_page(int page_n, Table* table) {
	if (table->pages[page_n] != NULL) {
		return;
	}

	table->pages[page_n] = malloc(PAGE_SIZE * RECORD_SIZE);
	fseek(table->db_file, (page_n * PAGE_SIZE * RECORD_SIZE), SEEK_SET);
	fread(table->pages[page_n], (PAGE_SIZE * RECORD_SIZE) , 1, table->db_file);
}


void save_table(Table *table) {
	fseek(table->db_file, 0, SEEK_SET);

	for (int i = 0 ; i < N_PAGES ; ++i) {
		if (table->pages[i] == NULL) {
			continue;
		}

		fseek(table->db_file, i * RECORD_SIZE * PAGE_SIZE, SEEK_SET);

		fwrite(table->pages[i], PAGE_SIZE * RECORD_SIZE, 1, table->db_file);

		free(table->pages[i]);
	}

	// Write n of elements in the end of the file
	char *n_of_elements_str = malloc(sizeof(char) * 10);
	sprintf(n_of_elements_str, "%d", table->n_records);
	fseek(table->db_file, ((table->n_records / PAGE_SIZE) + 1) * (RECORD_SIZE * PAGE_SIZE), SEEK_SET);
	fwrite(n_of_elements_str, strlen(n_of_elements_str), 1, table->db_file);

}

