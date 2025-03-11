#ifndef _SENDMAIL_H
#define _SENDMAIL_H

#include <stdbool.h>
#include <stdio.h>

struct MailInfo {
	char mail_name[128];
	char mail_from[128];
	char mail_path[128];
};

bool send_mail(struct MailInfo* info);
bool check_new_mail(struct MailInfo* info);
bool get_mail_from(struct MailInfo* info);
bool get_mail_info(struct MailInfo* info);

void reset_mail_info(struct MailInfo* info);

#endif

