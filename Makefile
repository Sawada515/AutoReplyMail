CC = gcc
SRCS = ./src/auto_reply.c ./src/main.c

CFLAG = -O2 -Wall -Wformat=2 -Wconversion -Wtrampolines -Wimplicit-fallthrough \
	-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 \
	-D_GLIBCXX_ASSERTIONS \
	-fstack-clash-protection -fstack-protector-strong \
	-Wl,-z,nodlopen -Wl,-z,noexecstack \
	-Wl,-z,relro -Wl,-z,now \
	-fPIE -pie -fPIC -Wmultichar -Wformat -Wformat-nonliteral -Wformat-security -Wformat-signedness

all:
	$(CC) ./src/check_config.c -W -Wall -Wconversion -Wtrampolines -Wimplicit-fallthrough -o check_config
	$(CC) -o AutoReplyMail $(CFLAG) $(SRCS)
	
	@touch /etc/systemd/system/AutoReply.service
	@echo "[Unit]" > /etc/systemd/system/AutoReply.service
	@echo "Description=AutoReply daemon" >> /etc/systemd/system/AutoReply.service
	@echo "" >> /etc/systemd/system/AutoReply.service
	@echo "[Service]" >> /etc/systemd/system/AutoReply.service
	@echo "ExecStartPre=/root/AutoReply/check_config" >> /etc/systemd/system/AutoReply.service
	@echo "ExecStart=/root/AutoReply/start_script.sh" >> /etc/systemd/system/AutoReply.service
	@echo "Restart=no" >> /etc/systemd/system/AutoReply.service
	@echo "Type=simple" >> /etc/systemd/system/AutoReply.service
	@echo "" >> /etc/systemd/system/AutoReply.service
	@echo "[Install]" >> /etc/systemd/system/AutoReply.service
	@echo "WantedBy=multi-user.target" >> /etc/systemd/system/AutoReply.service
	@echo "" >> /etc/systemd/system/AutoReply.service

	@systemctl enable AutoReply.service
	@echo "please restart AutoReply.service"

clean:
	rm -rf AutoReplyMail

build:
	$(CC) -o AutoReplyMail $(CFLAG) $(SRCS)
	@echo "build ok"

