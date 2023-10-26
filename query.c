#include "query.h"
#include "table.h"
#include "user.h"

QUERY *init_query() {
	QUERY *query = malloc(sizeof(QUERY));
	query->user_to_insert = malloc(sizeof(User));
	query->result = malloc(sizeof(char) * MAX_RESPONSE_SIZE);

	return query;
}
void execute_query(Table *table, QUERY query) {
	// size_t index;
	switch (query.type) {
		case SELECT:
			if (query.index >= table->n_records) {
				query.result_type = INVALID_INDEX_ERR;
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

			memset(query.result, 0, strlen(query.result)); // Clearing previous result
			strncpy(query.result, return_str, strlen(return_str));

			free(return_str);
			free(deserialized_user);
			free(serialized_user_str);

			query.result_type = SELECT_SUCCESS;

			break;
		case INSERT:
			if (table->n_records >= (N_PAGES * PAGE_SIZE)) {
				query.result_type = MAX_RECORDS_ERR;
				break;
			}

			User *user_to_insert = query.user_to_insert;
			
			void *serialized_user = serialize_user(user_to_insert);

			int page_number = (table->n_records) / PAGE_SIZE;
			int page_offs = (table->n_records) % PAGE_SIZE;

			load_page(page_number, table);
			void *page_bfr = table->pages[page_number];

			memcpy(page_bfr + (page_offs * RECORD_SIZE), serialized_user, RECORD_SIZE);

			memset(query.result, 0, strlen(query.result)); // Clearing previous result
			strncpy(query.result, serialized_user, strlen(serialized_user));

			free(serialized_user);

			table->n_records += 1;
		
			query.result_type = INSERT_SUCCESS;

			break;
		default:
			query.result_type = UNKNOWN_ERR;
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
