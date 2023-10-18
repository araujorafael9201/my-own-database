#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int RECORD_SIZE = 32;
int MAX_RECORDS = 10;
int MAX_QUERY_SIZE = 20;

typedef enum QUERY_TYPE {
	SELECT,
	INSERT,
	INVALID,
	EXIT
} QUERY_TYPE;

typedef struct QUERY {
	QUERY_TYPE type;
	char *command;
} QUERY;

typedef enum QUERY_RESULT_TYPE  {
	SELECT_SUCCESS,
	INVALID_INDEX_ERR,
	INSERT_SUCCESS,
	MAX_RECORDS_ERR,
	UNKNOWN_ERR,
} QUERY_RESULT_TYPE;

typedef struct QUERY_RESULT {
	QUERY_RESULT_TYPE type;
	char* result;
} QUERY_RESULT;

QUERY *init_query() {
	QUERY *query = (QUERY *) malloc(sizeof(QUERY));
	query->command = malloc(sizeof(char) * 15);

	return query;
}

QUERY_RESULT *init_query_result() {
	QUERY_RESULT *result = malloc(sizeof(QUERY_RESULT));
	result->result = malloc(sizeof(char) * 10);

	return result;
}

void execute_query(int n_records, char* db_buffer, QUERY query, QUERY_RESULT *query_result) {
	size_t index;
	switch (query.type) {
		case SELECT:
			index = strtol(query.command, (char **)NULL, 10);
		
			if (index >= n_records) {
				query_result->type = INVALID_INDEX_ERR;
				break;
			}

			memset(query_result->result, 0, strlen(query_result->result)); // Clearing previous result
			strncpy(query_result->result, db_buffer + (index * RECORD_SIZE), RECORD_SIZE);

			query_result->type = SELECT_SUCCESS;

			break;
		case INSERT:
			if (n_records >= MAX_RECORDS) {
				query_result->type = MAX_RECORDS_ERR;
				break;
			}

			strncpy(db_buffer + (n_records * RECORD_SIZE), query.command, strlen(query.command) - 1); // -1 removes \n

			memset(query_result->result, 0, strlen(query_result->result)); // Clearing previous result
			strncpy(query_result->result, query.command, strlen(query.command) - 1); // -1 removes \n
			
			query_result->type = INSERT_SUCCESS;

			break;
		default:
			query_result->type = UNKNOWN_ERR;
			break;
	}
}


void parse_query(char *query_buffer, QUERY *query) {
	if (strncmp("SELECT", query_buffer, 6) == 0) {
		query->type = SELECT;
		strcpy(query->command, query_buffer + 7);
	} else if (strncmp("INSERT", query_buffer, 6) == 0) {
		query->type = INSERT;	
		strcpy(query->command, query_buffer + 7);
	} else if (strncmp("EXIT", query_buffer, 4) == 0) {
		query->type = EXIT;	
	} else {
		query->type = INVALID;
	}
}

int main(int argc, char* argv[]) {
	FILE* database_file;
	database_file = fopen(argv[1], "rb+");

	// Get n of records
	fseek(database_file, 0, SEEK_END);
	int n_records = ftell(database_file) / RECORD_SIZE;
	fseek(database_file, 0, SEEK_SET);

	int db_total_size = RECORD_SIZE * MAX_RECORDS;
	char *db_buffer = malloc(db_total_size);
	fread(db_buffer, n_records * RECORD_SIZE, 1, database_file);

	char *query_text_buffer = malloc(MAX_QUERY_SIZE);
	QUERY *query = init_query();
	QUERY_RESULT *query_result = init_query_result();

	int stop = 0;

	while (stop == 0) {
		printf("Total n of records: %d\n", n_records);
		fgets(query_text_buffer, MAX_QUERY_SIZE, stdin);

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
				execute_query(n_records, db_buffer, *query, query_result);

				switch (query_result->type) {
					case INSERT_SUCCESS:
						printf("Inserted %s\n", query_result->result);
						n_records += 1;
						break;
					case SELECT_SUCCESS:
						printf("%s\n", query_result->result);
						break;
					case INVALID_INDEX_ERR:
						printf("Invalid Index: %s\n", query->command);
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

	fseek(database_file, 0, SEEK_SET);
	fwrite(db_buffer, n_records * RECORD_SIZE, 1, database_file);
	fclose(database_file);

	free(query_text_buffer);
	free(db_buffer);

	return 0;
}
