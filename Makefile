all: Host view insert 

CC = gcc
Host: host.c
	$(CC) -o Host host.c

view: view.c
	$(CC) -o view.cgi view.c

insert: insert.c
	$(CC) -o insert.cgi insert.c

.PHONY: clean,run
clean:
	rm Host view.cgi insert.cgi

test:
	./Host
