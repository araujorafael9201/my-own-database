#include "query.h"
#include "table.h"
#include "user.h"
#include <ctype.h>
#include <string.h>

QUERY *init_query() {
	QUERY *query = malloc(sizeof(QUERY));
	query->user_to_insert = malloc(sizeof(User));
	query->name_to_search = malloc(sizeof(char) * NAME_SIZE);
	query->result = malloc(sizeof(char) * MAX_RESPONSE_SIZE);

	return query;
}

void execute_query(Table *table, QUERY *query) {
	int page_n;
	int page_offset;
	void *page_buffer;
	char *serialized_user_str = malloc(RECORD_SIZE);
	char *return_str = malloc(RECORD_SIZE + 10);
	User deserialized_user;
	INDEX_ENTRY entry;
	int idx_position;
	void *entry_str;

	switch (query->type) {
		case SELECT_BY_INDEX:
			if (query->index >= table->n_records) {
				query->result_type = INVALID_INDEX_ERR;
				break;
			}

			page_n = query->index / PAGE_SIZE;
			page_offset = query->index % PAGE_SIZE;

			load_page(page_n, table);

			page_buffer = table->pages[page_n];

			memcpy(serialized_user_str, page_buffer + (page_offset * RECORD_SIZE), RECORD_SIZE);
			serialized_user_str[RECORD_SIZE] = '\0';

			deserialize_user(&deserialized_user, serialized_user_str);
		
			sprintf(return_str, "%s - %s - %d", deserialized_user.name, deserialized_user.email, deserialized_user.height);

			memset(query->result, 0, strlen(query->result)); // Clearing previous result
			strncpy(query->result, return_str, strlen(return_str));

			query->result_type = SELECT_SUCCESS;

			break;
		case SELECT_BY_NAME: 
				idx_position = hash((query->name_to_search));
				load_idx_entry(table->name_idx, idx_position);
				entry_str = table->name_idx->entries[idx_position];
		
				deserialize_entry(&entry, entry_str);
				page_n = entry.index / PAGE_SIZE;
				page_offset = entry.index % PAGE_SIZE;

				load_page(page_n, table);
				page_buffer = table->pages[page_n];
				
				memcpy(serialized_user_str, (page_buffer + (page_offset * RECORD_SIZE)), RECORD_SIZE);
				deserialize_user(&deserialized_user, serialized_user_str);

				memset(query->result, 0, strlen(query->result)); // Clearing previous result
				if (strcmp(query->name_to_search, deserialized_user.name) == 0) {
					sprintf(return_str, "%s - %s - %d", deserialized_user.name, deserialized_user.email, deserialized_user.height);

					strncpy(query->result, return_str, strlen(return_str));

					query->result_type = SELECT_SUCCESS;

				} else {
				sprintf(return_str, "%s", query->name_to_search);
				strncpy(query->result, return_str, strlen(return_str));

				query->result_type = SELECT_ERR;

				}
			break;
		case INSERT:
			if (table->n_records >= (N_PAGES * PAGE_SIZE)) {
				query->result_type = MAX_RECORDS_ERR;
				break;
			}

			User *user_to_insert = query->user_to_insert;
		
			// check if name is unique
			idx_position = hash(user_to_insert->name);
			load_idx_entry(table->name_idx, idx_position);
			entry_str = table->name_idx->entries[idx_position];
			deserialize_entry(&entry, entry_str);
		
			if (strcmp(entry.name, "") != 0) { // name exists
				memset(query->result, 0, strlen(query->result)); // Clearing previous result
				strncpy(query->result, query->user_to_insert->name, strlen(query->user_to_insert->name));
			
				query->result_type = INSERT_ERR_DUPLICATE;

				break;
			}

			serialize_user(user_to_insert, serialized_user_str);

			page_n = (table->n_records) / PAGE_SIZE;
			page_offset = (table->n_records) % PAGE_SIZE;

			insert_entry(table->name_idx, query->user_to_insert, table);

			load_page(page_n, table);
			page_buffer = table->pages[page_n];

			memcpy(page_buffer + (page_offset * RECORD_SIZE), serialized_user_str, RECORD_SIZE);

			memset(query->result, 0, strlen(query->result)); // Clearing previous result
			strncpy(query->result, serialized_user_str, strlen(serialized_user_str));

			table->n_records += 1;
		
			query->result_type = INSERT_SUCCESS;

			break;
		default:
			query->result_type = UNKNOWN_ERR;
			break;
	}


	free(serialized_user_str);
	free(return_str);
}


void parse_query(char *query_buffer, QUERY *query) {
	char *query_buffer_copy = malloc(strlen(query_buffer));
	strncpy(query_buffer_copy, query_buffer, strlen(query_buffer));

	char *token = strtok(query_buffer_copy, " ");

	if (strcmp("select", token) == 0) {
		token = strtok(NULL, " ");
		if (isdigit(*token)) {
			query->index = atoi(token);
			query->type = SELECT_BY_INDEX;
		} else {
			memset(query->name_to_search, 0, strlen(query->name_to_search)); // clear previous commit
			strncpy(query->name_to_search, token, strlen(token) - 1); // -1 to remove \n
			query->type = SELECT_BY_NAME;
		}
	} else if (strcmp("insert", token) == 0) {
		query->type = INSERT;	
		sscanf(query_buffer, "insert %s %s %d", query->user_to_insert->name, query->user_to_insert->email, &(query->user_to_insert->height));
	} else if (strncmp("exit", query_buffer, 4) == 0) {
		query->type = EXIT;	
	} else {
		query->type = INVALID;
	}

	free(query_buffer_copy);
}

void close_query(QUERY *query) {
	free(query->result);
	free(query->user_to_insert);
	free(query->name_to_search);
	free(query);
}
