#include <stdio.h>

#ifndef TABLE_H
#define TABLE_H

#define N_PAGES 10
#define PAGE_SIZE 3 // Records per page

typedef struct Table {
	FILE *db_file;
	int n_records;
	void *pages[N_PAGES];
} Table;

Table *init_table();
void load_page(int page_n, Table* table);
void save_table(Table *table);

#endif

