#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "table.h"
#include "user.h"
#include "query.h"

int MAX_QUERY_SIZE = 50;

int main(int argc, char* argv[]) {
	Table *table = init_table();

	if (argc == 1) {
		table->db_file = fopen("test.db", "rb+");
	} else {
		table->db_file = fopen(argv[1], "rb+");
	}

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

	char *query_text_buffer = malloc(MAX_QUERY_SIZE);

	QUERY *query = malloc(sizeof(QUERY));
	query->user_to_insert = malloc(sizeof(User));

	QUERY_RESULT *query_result = init_query_result();

	int stop = 0;

	while (stop == 0) {
		printf("> ");
		fgets(query_text_buffer, MAX_QUERY_SIZE, stdin);
	
		if (query_text_buffer[strlen(query_text_buffer) - 1] != '\n') {
			int ch;
			while ((ch = getchar()) != '\n' && ch != EOF) {
				// Clear the input buffer
			}
		}


		parse_query(query_text_buffer, query);

		switch (query->type) {
			case INVALID:
				printf("Invalid Query\n");
				break;
			case EXIT:
				printf("Exiting...\n");
				stop = 1;
				break;
			default:
				execute_query(table, *query, query_result);

				int n_pages = 0;
				for (int i = 0 ; i < N_PAGES ; ++i) {
					if (table->pages[i]) {
						n_pages +=1;	
					}
				}

				printf("N of pages loaded: %d\n", n_pages);

				switch (query_result->type) {
					case INSERT_SUCCESS:
						printf("Inserted %s\n", query_result->result);
						table->n_records += 1;
						break;
					case SELECT_SUCCESS:
						printf("%s\n", query_result->result);
						break;
					case INVALID_INDEX_ERR:
						printf("Invalid Index: %d\n", query->index);
						break;
					case MAX_RECORDS_ERR:
						printf("Database is full\n");
						break;
					default:
						break;
				}

				break;
		}
	}

	printf("Writing changes...\n");

	save_table(table);

	fclose(table->db_file);

	free(query);
	free(query_text_buffer);

	free(table);

	return 0;
}
