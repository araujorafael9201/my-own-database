#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum QUERY_TYPE {
	SELECT,
	INSERT,
	INVALID,
	EXIT
} QUERY_TYPE;

int execute_query(int *cur_pos, char* database[], QUERY_TYPE query_type) {
	printf("%d\n", *cur_pos);
	switch (query_type) {
		case SELECT:
			printf("Selecting...\n");
			for (int i = 0 ; i < *cur_pos ; ++i) {
				printf("%s\n", database[i]);
			}
			break;
		case INSERT:
			printf("Inserting...\n");
			database[*cur_pos] = "Rafael";
			*cur_pos += 1;
			break;
		case EXIT:
			printf("Exiting...\n");
			exit(1);
			break;
		case INVALID:
			printf("Invalid Operation.\n");
			return 0;
			break;
		default:
			return 0;
	}
	
	return 1;
}

int MAX_QUERY_SIZE = 20;

QUERY_TYPE parse_query(char *query_buffer) {
	if (strncmp("SELECT", query_buffer, 6) == 0) {
		return SELECT;
	}

	if (strncmp("INSERT", query_buffer, 6) == 0) {
		return INSERT;
	}

	if (strncmp("EXIT", query_buffer, 4) == 0) {
		return EXIT;
	}

	return INVALID;
}

int main(int argc, char* argv[]) {
	char *database[10];
	int* cur_pos = malloc(sizeof(int));
	*cur_pos = 0;

	char *query_buffer = malloc(MAX_QUERY_SIZE);
	QUERY_TYPE query_type;
	int query_response;

	while (1) {
		fgets(query_buffer,  MAX_QUERY_SIZE, stdin);
		query_type = parse_query(query_buffer);		

		query_response = execute_query(cur_pos, database, query_type);
	}

	return 0;
}
