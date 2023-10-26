#include "user.h"
#include "table.h"

#ifndef QUERY_H
#define QUERY_H

#define MAX_RESPONSE_SIZE 100

typedef enum QUERY_TYPE {
	SELECT,
	INSERT,
	INVALID,
	EXIT
} QUERY_TYPE;

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

QUERY_RESULT *init_query_result();
void execute_query(Table *table, QUERY query, QUERY_RESULT *query_result);
void parse_query(char *query_buffer, QUERY *query);


#endif
