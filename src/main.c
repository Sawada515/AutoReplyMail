#include "auto_reply.h"

#include <stdio.h>
#include <unistd.h>
#include <syslog.h>

int main(void)
{
	struct MailInfo mail = {{ '\0', '\0', '\0' }};

	bool has_mail_info = false;

	openlog("AutoReplyMail", LOG_ODELAY, LOG_USER);

	for (;;) {
		if(check_new_mail(&mail)) {
			has_mail_info = get_mail_info(&mail);
		}

		if (has_mail_info) {
			if (send_mail(&mail)) {
				syslog(LOG_INFO, "succsessful send mail\n");
			}
			else {
				syslog(LOG_INFO, "can not send mail\n");
			}

			reset_mail_info(&mail);

			has_mail_info = false;
		}

		sleep(1);
	}

	return 0;
}

