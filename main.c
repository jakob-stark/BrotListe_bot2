#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sqlite3.h>
//#include <regex.h>

#include <telebot.h>

/* declarations and type definitions */
void process_message(telebot_handler_t handle, telebot_message_t * message);

typedef enum {	COMMAND_INVALID = -1,
		COMMAND_START = 0,
		COMMAND_HILFE,
		COMMAND_ABBESTELLEN,
		COMMAND_BESTELLEN,
		COMMAND_KAUFEN,
		COMMAND_GEKAUFT,
		COMMAND_BEZAHLEN,
		COMMAND_MAX
} command_t;

/* constant strings */
const char * command_strings[COMMAND_MAX] = {
	"start",
	"hilfe",
	"abbestellen",
	"bestellen",
	"kaufen",
	"gekauft",
	"bezahlen"
};

const char * welcome_message = "WELCOME\n";

/* database */
const char * database_file = "database.db";
sqlite3 * db = NULL;

char query_string[1024];
int query_return;

int database_get_int(const char * sql);
void database_exec(const char * sql);


/* implementations */

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

	/* init database */
	if ( sqlite3_open(database_file, &db) != SQLITE_OK ) {
		printf("Error opening databasei\n");
		sqlite3_close(db);
		exit(1);
	}
	database_exec("CREATE TABLE IF NOT EXISTS Log(NachrichtId INT PRIMARY KEY, BenutzerId INT, Von TEXT, Datum INT, Befehl TEXT, Gekauft BIT)");

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

	const char * reply_text = NULL;
	switch ( command ) {
		case COMMAND_START:
		case COMMAND_HILFE:
			reply_text = "Ich zaehle Brote. Mit 'bestellen' kannst du dir eins  bestellen, mit 'abbestellen' bestellst du es wieder ab. Kapiert? \U0001f644\n";
			break;
		case COMMAND_BESTELLEN:
			snprintf(query_string, sizeof(query_string), "INSERT INTO Log VALUES(%d,%d,'%s',%d,'p',0)", message->message_id, message->from->id, message->from->first_name, message->date);
			database_exec(query_string);
			snprintf(query_string, sizeof(query_string), "Ein Broetchen mehr fuer %s", message->from->first_name);
			reply_text = query_string;
			break;
		case COMMAND_ABBESTELLEN:
			reply_text = "Geh und stirb, abbestellt wird hier nicht!";
			break;
		case COMMAND_KAUFEN:
			snprintf(query_string, sizeof(query_string), "SELECT Count(BenutzerId) FROM Log WHERE Datum > %d AND Gekauft=0", message->date - 24*3600);
			query_return = database_get_int(query_string);
			snprintf(query_string, sizeof(query_string), "Liebe(r) %s, heute muessen %d Broetchen gekauft werden", message->from->first_name, query_return);
			reply_text = query_string;
			break;
		case COMMAND_GEKAUFT:
			database_exec("UPDATE Log SET Gekauft=1");
			reply_text = "Naechstes mal bitte schneller! \xF0\x9F\x99\x84";
			break;
		case COMMAND_BEZAHLEN:
			snprintf(query_string, sizeof(query_string), "SELECT Count(BenutzerId) FROM Log WHERE BenutzerId=%d", message->from->id);
			query_return = database_get_int(query_string);
			snprintf(query_string, sizeof(query_string), "Liebe(r) %s, du musst %d Broetchen bezahlen", message->from->first_name, query_return);
			reply_text = query_string;
		default:
			reply_text = "\U0001f595";
			reply_text = NULL;
			break;
	}

	if ( reply_text != NULL ) {
		telebot_send_message(handle, message->chat->id, (char*)reply_text,
					NULL, false, false, 0, NULL);
	}
}

int database_get_int(const char * sql) {
	int result = 0;

	sqlite3_stmt * statement;
	if ( sqlite3_prepare_v2(db, sql, strlen(sql), &statement, NULL) != SQLITE_OK ) {
		sqlite3_finalize(statement);
		return -1;
	}

	int ret;
	ret = sqlite3_step(statement);
	if ( (ret == SQLITE_ROW) || (ret == SQLITE_DONE) ) {
		result = sqlite3_column_int(statement, 0);
	}
	sqlite3_finalize(statement);
	return result;
}

void database_exec(const char * sql) {
	sqlite3_stmt * statement;
	if ( sqlite3_prepare_v2(db, sql, strlen(sql), &statement, NULL) != SQLITE_OK ) {
		sqlite3_finalize(statement);
		return;
	}
	sqlite3_step(statement);
	sqlite3_finalize(statement);
}
