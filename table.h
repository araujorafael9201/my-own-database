#include <stdio.h>
#include "user.h"

#ifndef TABLE_H
#define TABLE_H

#define N_PAGES 1000
#define PAGE_SIZE 1000 // Records per page

typedef struct INDEX_ENTRY {
	char name[NAME_SIZE];
	int index;
} INDEX_ENTRY;

typedef struct NAME_INDEX {
	FILE *idx_file;
	void *entries[N_PAGES * PAGE_SIZE];
} NAME_INDEX;

typedef struct Table {
	FILE *db_file;
	int n_records;
	void *pages[N_PAGES];
	NAME_INDEX *name_idx;
} Table;

Table *init_table(char *db_file);
void load_page(int page_n, Table* table);
void save_table(Table *table);

NAME_INDEX *init_index(char *filename);
int hash(char *name);
void insert_entry(NAME_INDEX *name_idx, User *user, Table *table);
void load_idx_entry(NAME_INDEX *idx, int position);
void save_name_index(NAME_INDEX *idx);
void serialize_entry(INDEX_ENTRY *entry, char *serialized);
void deserialize_entry(INDEX_ENTRY *entry, char *serialized);

#endif

