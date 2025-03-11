/*my lib*/
#include "auto_reply.h"

/*system lib*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <syslog.h>
#include <regex.h>

/*config*/
#include "../.autoreply.config"

//#define DEBUG_MODE

#define MAX_BODY_STRING 9

#ifdef DEBUG_MODE
#define DEBUG() printf("%s\n", __func__)
#else
#define DEBUG() ;
#endif

static bool check_mail_address(const char *mail_addr);

bool send_mail(struct MailInfo* info)
{
	FILE *sendmail_p = NULL;
	char command[256] = { '\0' };

	DEBUG();	

	if (*(info->mail_from) == '\0') {
		syslog(LOG_INFO, "mail send faile");

		return false;
	}

	snprintf(command, sizeof(command), "sendmail -t %s", (info->mail_from));

	if(!check_mail_address(info->mail_from)) {
		syslog(LOG_WARNING, "unkown mail address");

		return false;
	}

	sendmail_p = popen(command, "w");
	if(sendmail_p == NULL){
		return false;
	}

	fprintf(sendmail_p, "From: %s\n", MY_MAIL_ADDRESS);
	fprintf(sendmail_p, "To: %s\n", info->mail_from);
	fprintf(sendmail_p, "Subject: %s\n", SUBJECT);

	for(signed char i = 0; i < MAX_BODY_STRING; ++i) {
		if(BODY_STRING[i] == NULL) {
			break;
		}
		else {
			fprintf(sendmail_p, "%s\n", BODY_STRING[i]);
		}
	}

	fprintf(sendmail_p, ".\n");

	pclose(sendmail_p);

	return true;
}

bool check_new_mail(struct MailInfo* info)
{
	FILE *ls_txt_fp = NULL;
	int system_result = 1;
	int ls_result = 0;

	char command[256] = { '\0' };

	snprintf(command, sizeof(command), "ls %s -1 > /root/AutoReply/ls.txt", TARGET_MAIL_DIR);

	DEBUG();	

	system_result = system(command);
	if(system_result == 1) {
		syslog(LOG_INFO, "system command is false in check_new_mail function");

		return false;
	}

	ls_txt_fp = fopen("/root/AutoReply/ls.txt", "r");
	if (ls_txt_fp == NULL) {
		return false;
	}
	ls_result = fscanf(ls_txt_fp, "%s", info->mail_name);

	fclose(ls_txt_fp);

	if (strchr(info->mail_name, '.') == NULL || ls_result == 0) {
		return false;
	}
	else {
		syslog(LOG_INFO, "get new mail");

		return true;
	}
}

bool get_mail_from(struct MailInfo* info)
{
	FILE* mail_file_fp = NULL;
	char mail_path[256] = { '\0' };
	int get_from = 0;
	char command[512] = { '\0' };

	DEBUG();	

	snprintf(mail_path, sizeof(mail_path), "%s%s", TARGET_MAIL_DIR, info->mail_name);

	mail_file_fp = fopen(mail_path, "r");
	if(mail_file_fp == NULL) {
		syslog(LOG_INFO, "no such file or directory");

		return false;
	}
	get_from = fscanf(mail_file_fp, "Return-Path: <%s>", info->mail_from);
	if(get_from == 0) {
		syslog(LOG_WARNING, "can't get Return-Path");

		return false;
	}

	fclose(mail_file_fp);

	if(*(info->mail_from) == '-') {
		reset_mail_info(info);

		char rm_command[256] = { '\0' };
		snprintf(rm_command, sizeof(rm_command), "rm %s*", TARGET_MAIL_DIR);

		if(system(rm_command));

		syslog(LOG_WARNING, "get unknown mail");

		return false;
	}

	char *p = strchr(info->mail_from, '>');
	if(p == NULL){
		;
	}
	else{
		*p = '\0';
	}

	DEBUG();

	if (*(info->mail_from) == '\0') {
		syslog(LOG_INFO, "get from faile");

		return false;
	}
	else {
		snprintf(command, sizeof(command), "mv %s %s", mail_path, MOV_MAIL_DIR);

		if(system(command));

		syslog(LOG_INFO, "New mail have been moved");

		return true;
	}
}

bool get_mail_info(struct MailInfo* info)
{
	bool mail_from_state = false;

	DEBUG();	

	mail_from_state = get_mail_from(info);
	if(mail_from_state == false) {
		syslog(LOG_INFO, "mail info is nothing\n");

		return false;
	}
	else {
		return true;
	}
}

void reset_mail_info(struct MailInfo* info)
{
	DEBUG();	

	memset(info->mail_from, '\0', 128);
	memset(info->mail_name, '\0', 128);
	memset(info->mail_path, '\0', 128);
}

static bool check_mail_address(const char *mail_addr)
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

