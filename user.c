#include "user.h"

char *serialize_user(User *user) {
	char *serialized_user = (char *)malloc(sizeof(char) * RECORD_SIZE);

	char *char_height = (char *)malloc(sizeof(char) * 10);
	sprintf(char_height, "%d", user->height);

	memcpy(serialized_user, user->name, NAME_SIZE);
	memcpy(serialized_user + NAME_SIZE, user->email, EMAIL_SIZE);
	memcpy(serialized_user + NAME_SIZE + EMAIL_SIZE + HEIGHT_STR_SIZE - strlen(char_height), char_height, strlen(char_height));

	free(char_height);
	return serialized_user;
}

User *deserialize_user(char *serialized_user) {
	User *user = (User *)malloc(sizeof(User));

	strncpy(user->name, serialized_user, NAME_SIZE);
	strncpy(user->email, serialized_user + NAME_SIZE , EMAIL_SIZE);

	char *height = (char *)malloc(10);
	
	memcpy(height,serialized_user + NAME_SIZE + EMAIL_SIZE, 10);

	int n_length;
	for (int i = 0 ; i < 10 ; ++i) {
		if (height[i]) {
			n_length = 10 - i;
			break;
		} 
	}

	char *height_trimmed = (char *)malloc(n_length);
	strncpy(height_trimmed, height + 10 - n_length, n_length);

	user->height = atoi(height_trimmed);

	free(height);
	free(height_trimmed);
	return user;
}
