#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"
#include "user.h"

NAME_INDEX *init_index(char *filename) {
	NAME_INDEX *idx = malloc(sizeof(NAME_INDEX));

	idx->idx_file = fopen(filename, "rb+");

	return idx;
}

void load_idx_entry(NAME_INDEX *idx, int position) {
	if (idx->entries[position] != NULL) {
		return;
	}
	idx->entries[position] = malloc(NAME_SIZE + 10);
	fseek(idx->idx_file, (position * (NAME_SIZE + 10)), SEEK_SET);
	fread(idx->entries[position], (NAME_SIZE + 10), 1, idx->idx_file);

}

int hash(char *name) {
	int idx = 0;

	for (int i = 0 ; i < strlen(name) ; ++i) {
		idx += name[i];
	}

	idx = idx % (PAGE_SIZE * N_PAGES);

	return idx;
}

void insert_entry(NAME_INDEX *name_idx, User *user, Table *table) {
	int index = hash(user->name);

	load_idx_entry(name_idx, index);
	INDEX_ENTRY entry;
	strncpy(entry.name, user->name, strlen(user->name));
	entry.index = table->n_records;

	char serialized[NAME_SIZE + 10];
	serialize_entry(&entry, serialized);

	memcpy(name_idx->entries[index], serialized, NAME_SIZE + 10);
}

void deserialize_entry(INDEX_ENTRY *entry ,char *serialized) {
	strncpy(entry->name, serialized, NAME_SIZE);

	char position_str[10];
	strncpy(position_str, serialized + NAME_SIZE, 10);
	
	entry->index = atoi(position_str);
}

void serialize_entry(INDEX_ENTRY *entry, char *serialized) {
	memcpy(serialized, entry->name, strlen(entry->name));

	char position_str[10];
	sprintf(position_str, "%.9d", entry->index);

	memcpy(serialized + NAME_SIZE, position_str, 10);
}

void save_name_index(NAME_INDEX *idx) {
	int size = NAME_SIZE + 10;
	for (int i = 0 ; i < (PAGE_SIZE * N_PAGES) ; ++i) {
		if (idx->entries[i] != NULL) {
			// memset(serialized_idx_entry, 0, strlen(serialized_idx_entry));
			fseek(idx->idx_file, (size * i), SEEK_SET);
			fwrite(idx->entries[i], size, 1, idx->idx_file);

			free(idx->entries[i]);
		}
	}

	fclose(idx->idx_file);
}

Table *init_table(char *db_name) {
	char *db_file = malloc(strlen(db_name) + 5);
	char *idx_file = malloc(strlen(db_name) + 5);

	sprintf(db_file, "%s.db", db_name);
	sprintf(idx_file, "%s.idx", db_name);

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

	// Load index
	NAME_INDEX *name_idx = init_index(idx_file);
	table->name_idx = name_idx;

	free(db_file);
	free(idx_file);
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

	save_name_index(table->name_idx);
	free(table->name_idx);

	// Write n of elements in the end of the file
	char *n_of_elements_str = malloc(sizeof(char) * 10);
	sprintf(n_of_elements_str, "%d", table->n_records);
	fseek(table->db_file, ((table->n_records / PAGE_SIZE) + 1) * (RECORD_SIZE * PAGE_SIZE), SEEK_SET);
	fwrite(n_of_elements_str, strlen(n_of_elements_str), 1, table->db_file);

	free(n_of_elements_str);
}


