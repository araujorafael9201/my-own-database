#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N_PAGES 10 // N of pages
#define PAGE_SIZE 3 // Records per page

int MAX_QUERY_SIZE = 50;

#define NAME_SIZE 32
#define EMAIL_SIZE 20
#define HEIGHT_STR_SIZE 10 // User.height will occupy 10 characters when serialized
						   
#define MAX_RESPONSE_SIZE 100

typedef struct User {
	char name[NAME_SIZE];
	char email[EMAIL_SIZE];
	int height;
} User;

int RECORD_SIZE = NAME_SIZE + EMAIL_SIZE + HEIGHT_STR_SIZE;

typedef enum QUERY_TYPE {
	SELECT,
	INSERT,
	INVALID,
	EXIT
} QUERY_TYPE;

typedef struct Table {
	FILE *db_file;
	int n_records;
	void *pages[N_PAGES];
} Table;

Table *init_table() {
	Table *table = malloc(sizeof(Table));
	table->n_records = 0;

	return table;
}

typedef struct QUERY {
	QUERY_TYPE type;
	int index; // Only used in SELECT
	User *user_to_insert; // Only used in INSERT
} QUERY;

typedef enum QUERY_RESULT_TYPE  {
	SELECT_SUCCESS,
	INVALID_INDEX_ERR,
	INSERT_SUCCESS,
	INSERT_ERR,
	MAX_RECORDS_ERR,
	UNKNOWN_ERR,
} QUERY_RESULT_TYPE;

typedef struct QUERY_RESULT {
	QUERY_RESULT_TYPE type;
	char* result;
} QUERY_RESULT;

QUERY_RESULT *init_query_result() {
	QUERY_RESULT *result = malloc(sizeof(QUERY_RESULT));
	result->result = malloc(MAX_RESPONSE_SIZE) ;
	return result;
}

char *serialize_user(User *user) {
	char *serialized_user = malloc(sizeof(char) * RECORD_SIZE);

	char *char_height = malloc(sizeof(char) * 10);
	sprintf(char_height, "%d", user->height);

	memcpy(serialized_user, user->name, NAME_SIZE);
	memcpy(serialized_user + NAME_SIZE, user->email, EMAIL_SIZE);
	memcpy(serialized_user + NAME_SIZE + EMAIL_SIZE + HEIGHT_STR_SIZE - strlen(char_height), char_height, strlen(char_height));

	free(char_height);
	return serialized_user;
}

User *deserialize_user(char *serialized_user) {
	User *user = malloc(sizeof(User));

	strncpy(user->name, serialized_user, NAME_SIZE);
	strncpy(user->email, serialized_user + NAME_SIZE , EMAIL_SIZE);

	char *height = malloc(10);
	
	memcpy(height,serialized_user + NAME_SIZE + EMAIL_SIZE, 10);

	int n_length;
	for (int i = 0 ; i < 10 ; ++i) {
		if (height[i]) {
			n_length = 10 - i;
			break;
		} 
	}

	char *height_trimmed = malloc(n_length);
	strncpy(height_trimmed, height + 10 - n_length, n_length);

	user->height = atoi(height_trimmed);

	free(height);
	free(height_trimmed);
	return user;
}

void load_page(int page_n, Table* table) {
	if (table->pages[page_n] != NULL) {
		return;
	}
	table->pages[page_n] = malloc(PAGE_SIZE * RECORD_SIZE);
	fseek(table->db_file, (page_n * PAGE_SIZE * RECORD_SIZE), SEEK_SET);
	fread(table->pages[page_n], (PAGE_SIZE * RECORD_SIZE) , 1, table->db_file);
}

void execute_query(Table *table, QUERY query, QUERY_RESULT *query_result) {
	size_t index;
	switch (query.type) {
		case SELECT:
			if (query.index >= table->n_records) {
				query_result->type = INVALID_INDEX_ERR;
				break;
			}

			int page_n = query.index / PAGE_SIZE;
			int page_offset = query.index % PAGE_SIZE;

			load_page(page_n, table);

			void *page_buffer = table->pages[page_n];

			char *serialized_user_str = malloc(sizeof(char) * RECORD_SIZE + 1);
			memcpy(serialized_user_str, page_buffer + (page_offset * RECORD_SIZE), RECORD_SIZE);
			serialized_user_str[RECORD_SIZE] = '\0';

			User *deserialized_user;
			deserialized_user = deserialize_user(serialized_user_str);
		
			char *return_str = malloc(sizeof(char) * RECORD_SIZE + 10);
			sprintf(return_str, "%s - %s - %d", deserialized_user->name, deserialized_user->email, deserialized_user->height);

			memset(query_result->result, 0, strlen(query_result->result)); // Clearing previous result
			strncpy(query_result->result, return_str, strlen(return_str));

			free(return_str);
			free(deserialized_user);
			free(serialized_user_str);

			query_result->type = SELECT_SUCCESS;

			break;
		case INSERT:
			if (table->n_records >= (N_PAGES * PAGE_SIZE)) {
				query_result->type = MAX_RECORDS_ERR;
				break;
			}

			User *user_to_insert = query.user_to_insert;
			
			void *serialized_user = serialize_user(user_to_insert);

			int page_number = (table->n_records) / PAGE_SIZE;
			int page_offs = (table->n_records) % PAGE_SIZE;

			load_page(page_number, table);
			void *page_bfr = table->pages[page_number];

			memcpy(page_bfr + (page_offs * RECORD_SIZE), serialized_user, RECORD_SIZE);

			memset(query_result->result, 0, strlen(query_result->result)); // Clearing previous result
			strncpy(query_result->result, serialized_user, strlen(serialized_user));

			free(serialized_user);
		
			query_result->type = INSERT_SUCCESS;

			break;
		default:
			query_result->type = UNKNOWN_ERR;
			break;
	}
}


void parse_query(char *query_buffer, QUERY *query) {
	if (strncmp("select", query_buffer, 6) == 0) {
		query->type = SELECT;
		sscanf(query_buffer, "select %d", &query->index);
	} else if (strncmp("insert", query_buffer, 6) == 0) {
		query->type = INSERT;	
		sscanf(query_buffer, "insert %s %s %d", query->user_to_insert->name, query->user_to_insert->email, &(query->user_to_insert->height));
	} else if (strncmp("exit", query_buffer, 4) == 0) {
		query->type = EXIT;	
	} else {
		query->type = INVALID;
	}
}

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


	fclose(table->db_file);

	free(query);
	free(query_text_buffer);

	free(table);

	return 0;
}
