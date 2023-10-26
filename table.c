#include <stdio.h>
#include <stdlib.h>
#include "table.h"
#include "user.h"

Table *init_table(char *db_file) {
	Table *table = malloc(sizeof(Table));
	table->db_file = fopen(db_file, "rb+");

	// Get n of records
	fseek(table->db_file, 0, SEEK_END);
	if (ftell(table->db_file) == 0) {
		table->n_records = 0;
	} else {
		fseek(table->db_file, -10, SEEK_END);
		char* n_records = malloc(sizeof(char) * 10);
		fread(n_records, 10, 1, table->db_file);

		int n_records_str_len = 0;
		for (int i = 0 ; i < 10 ; ++i) {
			if (n_records[i]) {
				n_records_str_len += 1;
			}
		}
		
		char *n_records_trimmed = malloc(n_records_str_len);
		memcpy(n_records_trimmed, n_records + 10 - n_records_str_len, n_records_str_len);

		table->n_records = atoi(n_records_trimmed);

		free(n_records);
		free(n_records_trimmed);
	}

	fseek(table->db_file, 0, SEEK_SET);
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

