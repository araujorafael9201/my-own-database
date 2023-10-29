#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "table.h"
#include "user.h"
#include "query.h"

int MAX_QUERY_SIZE = 50;

int main(int argc, char* argv[]) {
	Table *table;
	if (argc >= 2) {
		table = init_table(argv[1]);
	} else {
		table = init_table("test");
	}

	char *query_text_buffer = malloc(MAX_QUERY_SIZE);

	QUERY *query = init_query();

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
				execute_query(table, query);

				int n_pages = 0;
				for (int i = 0 ; i < N_PAGES ; ++i) {
					if (table->pages[i]) {
						n_pages +=1;	
					}
				}

				printf("N of pages loaded: %d\n", n_pages);

				switch (query->result_type) {
					case INSERT_SUCCESS:
						printf("Inserted %s\n", query->result);
						break;
					case INSERT_ERR:
						printf("Error inserting %s\n", query->result);
						break;
					case INSERT_ERR_DUPLICATE:
						printf("%s already exists\n", query->result);
						break;
					case SELECT_SUCCESS:
						printf("%s\n", query->result);
						break;
					case SELECT_ERR:
						printf("Could not find %s\n", query->result);
						break;
					case INVALID_INDEX_ERR:
						printf("Invalid Index: %d\n", query->index);
						break;
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

	close_query(query);
	free(query_text_buffer);

	free(table);

	return 0;
}
