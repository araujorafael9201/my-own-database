#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef USER_H
#define USER_H

#define NAME_SIZE 32
#define EMAIL_SIZE 20
#define HEIGHT_STR_SIZE 10 // User.height will occupy 10 characters when serialized

#define RECORD_SIZE (NAME_SIZE + EMAIL_SIZE + HEIGHT_STR_SIZE)

typedef struct User {
	char name[NAME_SIZE];
	char email[EMAIL_SIZE];
	int height;
} User;

void serialize_user(User *user, char *serialized_user_str);
void deserialize_user(User *user, char *serialized_user);

#endif
