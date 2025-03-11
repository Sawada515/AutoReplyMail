#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <syslog.h>
#include <regex.h>

#define CREATE_CONFIG_FILE "/root/AutoReply/.autoreply.config"
#define CHECK_CONFIG_FILE  "/root/AutoReply/autoreply.config"

#define MAX_CONFIG_LINE 13

enum config { MY_MAIL_ADDRESS, TARGET_MAIL_DIR, MOV_MAIL_DIR, SUBJECT,
				BODY_STRING1, BODY_STRING2, BODY_STRING3, BODY_STRING4,
				BODY_STRING5, BODY_STRING6, BODY_STRING7, BODY_STRING8,
				BODY_STRING9,
};

const char config_string[MAX_CONFIG_LINE][32] = { "my_mail_address", "target_mail_dir", "mov_mail_dir",	"subject", "body_string1",
													"body_string2", "body_string3", "body_string4", "body_string5", "body_string6",
													"body_string7", "body_string8", "body_string9",
};

char read_config[MAX_CONFIG_LINE][255];

void fgetns(FILE *fp, char *buf, int string_len);

bool check_mail_address(const char *mail_addr);
bool check_dir_path(const char *path);

bool get_body_string(const char *string, char *dest);
void set_body_string(int body_count, const char *source, char *dest);

void create_config_file(void);

int main(void)
{
	FILE *config_file = NULL;

	char *config_str_p;

	openlog("AutoReplyMail check config", LOG_ODELAY, LOG_USER);

	config_file = fopen(CHECK_CONFIG_FILE, "r");
	if(config_file == NULL) {
		syslog(LOG_ERR, "can not open file");		

		perror("file open error");

		return 1;
	}

	for(signed char i = 0; i < MAX_CONFIG_LINE; ++i) {
		fgetns(config_file, read_config[i], 256 + 32);	

		if(strstr(config_string[i], read_config[i]) != NULL) {
			syslog(LOG_ERR, "config error %hhd %s", i, config_string[i]);

			return 1;
		}

		config_str_p = strchr(read_config[i], '=');
		if(config_str_p == NULL) {
			syslog(LOG_ERR, "config_str_p = NULL");

			return 1;
		}
		config_str_p++; //skip '='

		if(*config_str_p == ' ') {
			config_str_p++;
		}

		switch(i) {
			case MY_MAIL_ADDRESS:
				if(!check_mail_address(config_str_p)) {
					fprintf(stderr, "bad mail address : %s\n", config_str_p);

					syslog(LOG_ERR, "config error %hhd %s", MY_MAIL_ADDRESS, config_string[MY_MAIL_ADDRESS]);

					return 1;
				}
				
				strncpy(read_config[MY_MAIL_ADDRESS], config_str_p, strlen(config_str_p) + 1);

				break;
			case TARGET_MAIL_DIR:
				if(!check_dir_path(config_str_p)) {
					syslog(LOG_ERR, "config error %hhd %s", TARGET_MAIL_DIR, config_string[TARGET_MAIL_DIR]);

					return 1;
				}

				strncpy(read_config[TARGET_MAIL_DIR], config_str_p, strlen(config_str_p) + 1);

				break;
			case MOV_MAIL_DIR:
				if(!check_dir_path(config_str_p)) {
					syslog(LOG_ERR, "config error %hhd %s", MOV_MAIL_DIR, config_string[MOV_MAIL_DIR]);

					return 1;
				}

				strncpy(read_config[MOV_MAIL_DIR], config_str_p, strlen(config_str_p) + 1);

				break;
			case SUBJECT:
				if(*(config_str_p) == ' ') {
					config_str_p++;
				}

				strncpy(read_config[SUBJECT], config_str_p, strlen(config_str_p) + 1);

				break;
			default:
				static unsigned char body_string_count = BODY_STRING1;

				set_body_string(body_string_count, config_str_p, read_config[body_string_count]);

				body_string_count += 1;

				break;
		}
	}

	fclose(config_file);

	create_config_file();

	return 0;
}

void fgetns(FILE *fp, char *buf, int string_len)
{
	int get_ch;
	int count = 0;

	while(((get_ch = fgetc(fp)) != '\n') && string_len > count) {
		*(buf + count) = (char)get_ch;

		count += 1;
	}

	*(buf + (count + 1)) = '\0';
}

bool check_mail_address(const char *mail_addr)
{
	const char pattern[] = "^[a-zA-Z0-9]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"; /*Not RFC compliant*/

	regex_t regex;
	int result;

	if(regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB) != 0) {
		perror("regcomp faile");

		return false;
	}

	result = regexec(&regex, mail_addr, 0, NULL, 0);

	regfree(&regex);

	return result ? false : true;
}

bool check_dir_path(const char *path)
{
	FILE *ls_p = NULL;
	char command[512] = { '\0' };

	snprintf(command, sizeof(command), "ls %s", path);

	ls_p = popen(command, "r");

	if(ls_p == NULL) {
		perror("no such file or directory");

		return false;
	}
	else {
		pclose(ls_p);

		return true;
	}
}

bool get_body_string(const char *string, char *dest)
{
	unsigned int count = 0;
	const char *buf;

	if(*string == '"') {
		string++;
	}

	buf = strchr(string, '"');
	buf++;

	count = (unsigned int)(buf - string) - 1;
	if(count == 0) {
		return false;
	}
	else {
		strncpy(dest, string, count);
		*(dest + (count)) = '\0';

		return true;
	}
}

void set_body_string(int body_count, const char *source, char *dest)
{
	if(!get_body_string(source, dest)) {
		read_config[body_count][0] = '\0';
	}
}

void create_config_file(void)
{
	FILE *config_file = NULL;

	FILE_OPEN_RETRY:
	config_file = fopen(CREATE_CONFIG_FILE, "w");
	if(config_file == NULL) {
		perror("file open");

		goto FILE_OPEN_RETRY;
	}
	
	fprintf(config_file, "const char MY_MAIL_ADDRESS[] = \"%s\";\n", read_config[MY_MAIL_ADDRESS]);
	fprintf(config_file, "const char TARGET_MAIL_DIR[] = \"%s\";\n", read_config[TARGET_MAIL_DIR]);
	fprintf(config_file, "const char MOV_MAIL_DIR[] = \"%s\";\n", read_config[MOV_MAIL_DIR]);
	fprintf(config_file, "const char SUBJECT[] = \"%s\";\n", read_config[SUBJECT]);
	fprintf(config_file, "const char *BODY_STRING[10] = {\n");		/* *BODY_STRING[10] <= auto_repry.c for 0 -> NULL*/

	for(int i = BODY_STRING1; i <= BODY_STRING9; ++i) {
		if(read_config[i][0] == '\0') {
			fprintf(config_file, "\t\t%s,\n", "NULL");
		}
		else {
			fprintf(config_file, "\t\t\"%s\",\n", read_config[i]);
		}
	}

	fprintf(config_file, "\t\t%s\n", "NULL");
	fprintf(config_file, "};\n");

	fclose(config_file);
}

