#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sqlite3.h>
//#include <regex.h>

#include <telebot.h>

void process_message(telebot_handler_t handle, telebot_message_t * message);

typedef enum {	COMMAND_INVALID = -1,
		COMMAND_HILFE = 0,
		COMMAND_BESTELLEN,
		COMMAND_ABBESTELLEN,
		COMMAND_KAUFEN,
		COMMAND_GEKAUFT,
		COMMAND_MAX
} command_t;

const char * command_strings[COMMAND_MAX] = {
	"hilfe",
	"bestellen",
	"abbestellen",
	"kaufen",
	"gekauft"
};

const char * welcome_message = "WELCOME\n";

int main(int argc, char *argv[]) {
	printf(welcome_message);
	FILE *fp = fopen(".token","r");
	if ( fp == NULL ) {
		printf("Failed to open token file\n");
		exit(1);
	}

	char token[46];
	if ( fscanf(fp, "%45s", token) == 0 ) {
		printf("Reading token failed");
		fclose(fp);
		exit(1);
	}
	printf("Using token: %s\n", token);
	fclose(fp);

	telebot_handler_t handle;
	if ( telebot_create(&handle, token) != TELEBOT_ERROR_NONE) {
		printf("Telebot create failed\n");
		exit(1);
	}

	int index, count, offset = 1;
	telebot_error_e ret;

	while (1) {

		telebot_update_t *updates;
		ret = telebot_get_updates(handle, 0, 20, 20, NULL, 0, &updates, &count);
		if (ret != TELEBOT_ERROR_NONE) {
			printf("Error getting updates. Trying again...\n");
			continue;
		}

		for (index = 0; index < count; index++) {
			if ( updates[index].update_type == UPDATE_TYPE_MESSAGE ) {
				process_message(handle, &(updates[index].message));
			}
		}

		telebot_free_updates(updates, count);
	}

	telebot_destroy(handle);

	return(0);
}


void process_message(telebot_handler_t handle, telebot_message_t * message) {
	if ( message->text == NULL )
		return;

	int index;
	command_t command = COMMAND_INVALID;
	for ( index = 0; index < COMMAND_MAX; index++ ) {
		if ( strstr(message->text, command_strings[index] ) ) {
			command = index;
			break;
		}
	}
	switch ( command ) {
		case COMMAND_HILFE:
			telebot_send_location(handle, message->chat->id, 46.96, 10.19, false, 0, "");
			break;
		case COMMAND_BESTELLEN:
			break;
		case COMMAND_ABBESTELLEN:
			break;
		case COMMAND_KAUFEN:
			break;
		case COMMAND_GEKAUFT:
			break;
		default:
			
			break;
	}
}

