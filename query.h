#include "user.h"
#include "table.h"

#ifndef QUERY_H
#define QUERY_H

#define MAX_RESPONSE_SIZE 100

typedef enum QUERY_TYPE {
	SELECT_BY_INDEX,
	SELECT_BY_NAME,
	INSERT,
	INVALID,
	EXIT
} QUERY_TYPE;

typedef enum QUERY_RESULT_TYPE  {
	SELECT_SUCCESS,
	SELECT_ERR,
	INVALID_INDEX_ERR,
	INSERT_SUCCESS,
	INSERT_ERR,
	MAX_RECORDS_ERR,
	UNKNOWN_ERR,
} QUERY_RESULT_TYPE;

typedef struct QUERY {
	QUERY_TYPE type;
	int index; // Only used in SELECT by ID
	char *name_to_search; // Only used in SELECT by NAME
	User *user_to_insert; // Only used in INSERT
	QUERY_RESULT_TYPE result_type;
	char *result;
} QUERY;

QUERY *init_query();
void execute_query(Table *table, QUERY *query);
void parse_query(char *query_buffer, QUERY *query);
void close_query(QUERY *query);


#endif
