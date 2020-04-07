#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define DEBUG
#define SERVER_PORT "8080"
#define BUF_SIZE 2048
#define DATA_LENGTH 100

/*
insert.c    ---> CGI GET/POST insert program
view.c      ---> CGI View database.txt (local storage) program
Host.c      ---> main program
*/

char httpHeader_200[] = 
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n";

char httpHeader_404[] = 
"HTTP/1.1 404 Not Found\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n";

// to read file context into buffer
char* readFile(char* buffer, char* filename);

int main() {

      int input[2];
      int output[2];
      int status;
      pid_t cpid;

      // socket setting
      struct sockaddr_in server_addr, client_addr;
      socklen_t sin_len = sizeof(client_addr);  // length of client socket
      int fd_server, fd_client;                 // file descriptor
      char buf[BUF_SIZE];
	int on = 1;

      struct addrinfo hints, *res;

      memset(&hints, 0, sizeof hints);          // ensure struct is clean
      hints.ai_family = AF_UNSPEC;              // use IPv4 or IPv6, whichever
      hints.ai_socktype = SOCK_STREAM;          // TCP stream sockets
      hints.ai_flags = AI_PASSIVE;              // fill in my IP for me

      getaddrinfo(NULL, SERVER_PORT, &hints, &res);

      // create a socket
      fd_server = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
      if (fd_server < 0){
		perror("socket");
		exit(1);
	}
      // socket bind to port of getaddrinfo()
      if (bind(fd_server, res->ai_addr, res->ai_addrlen) == -1) {
		perror("bind");
		close(fd_server);
		exit(1);
	}
      // socket listen
	if (listen(fd_server,10) == -1) {
		perror("listen");
		close(fd_server);
		exit(1);
	}

      while (1) {
            // accept new request
            fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &sin_len);
		if (fd_client == -1) {
			perror("connection failed");
			continue;
		}

            printf("=== Got client connection ===\n");
            /* Use pipe to create a data channel betweeen two process
            'input'  handle data from 'parent process' to 'child process'
            'output' handle data from 'child process'  to 'parent process'*/
            if (pipe(input) < 0) {
                  perror("pipe");
                  exit(EXIT_FAILURE);
            }
            if (pipe(output) < 0) {
                  perror("pipe");
                  exit(EXIT_FAILURE);
            }

            // creates a new process to execute cgi program or send static files 
            if ((cpid = fork()) < 0) {
                  perror("fork");
                  exit(EXIT_FAILURE);
            }
            /* child process */
            if (cpid == 0) {
                  close(fd_server);
                  memset(buf, 0, BUF_SIZE);
                  read(fd_client, buf, BUF_SIZE - 1);
                  printf("=== Client info ===\n");
                  printf("%s\n", buf);

                  //close unused fd
                  close(input[1]);
                  close(output[0]);

                  //redirect the input from stdin to input
                  dup2(input[0], STDIN_FILENO); 

                  //redirect the output from stdout to output
                  dup2(output[1], STDOUT_FILENO);

                  //after redirect we don't need the old fd 
                  close(input[0]);
                  close(output[1]);

                  // for to parse insert text
                  char method[4];
                  char path[DATA_LENGTH + 20];
                  char data[DATA_LENGTH];

                  // for to read file context into buffer
                  char* buffer;               
                  
                  // request parser
                  if (sscanf(buf, "%s %s", method, path)) { 
                        // open index.html
                        if (!strcmp(path, "/")) {
                              puts(httpHeader_200);
                              buffer = readFile(buffer, "index.html");
                              puts(buffer);
                              free(buffer);
                        }
                        // insert.cgi program
                        else if (!strncmp(path, "/insert.cgi", strlen("/insert.cgi"))) {
                              // GET method: to split the data in url then excute to insert.cgi
                              if (!strcmp(method, "GET")) {
                                    int prefixLength = strlen("/insert.cgi?txtData=");
                                    strncpy(data, path + prefixLength , strlen(path) - prefixLength);
                              }
                              // POST method: to split the data in last line then excute to insert.cgi
                              else if (!strcmp(method, "POST")) {
                                    char *result = strstr(buf, "txtData=");
                                    int position = result - buf;
                                    int prefixLength = strlen("txtData=");
                                    strncpy(data, buf + position + prefixLength, strlen(buf) - position - prefixLength);
                              }
                              execlp("./insert.cgi", "./insert.cgi", data, NULL);
                        }
                        // excute view.cgi
                        else if (!strncmp(path, "/view.cgi", strlen("/view.cgi"))) {
                              execlp("./view.cgi", "./view.cgi", NULL);
                        }
                        // error handling return 404 not found
                        else {
                              puts(httpHeader_404);
                              buffer = readFile(buffer, "404.html");
                              puts(buffer);
                              free(buffer);
                        }
                  }
                  exit(0);
            }

            /* parent process */
            else { 
                  //close unused fd
                  close(input[0]);
                  close(output[1]);

                  char c;
                  // receive the message from the stdout then write into socket
                  while (read(output[0], &c, 1) > 0) write(fd_client, &c, 1);

                  send(fd_client, "\n", 1, 0);

                  // connection finish
                  close(input[1]);
                  close(output[0]);
                  close(fd_client);
                  waitpid(cpid, &status, 0);
            }
      }
}

char* readFile(char* buffer, char* filename) {

      FILE* infile;
      long numbytes;

      /* open an existing file for reading */
      infile = fopen(filename, "r");

      if(infile == NULL) exit(EXIT_FAILURE);;

      /* Get the number of bytes */
      fseek(infile, 0L, SEEK_END);
      numbytes = ftell(infile);

      /* reset the file position indicator to the beginning of the file */
      fseek(infile, 0L, SEEK_SET);	
      
      /* grab sufficient memory for the buffer to hold the text */
      buffer = (char*)calloc(numbytes, sizeof(char));	
      
      /* memory error */
      if(buffer == NULL) exit(EXIT_FAILURE);
      
      /* copy all the text into the buffer */
      fread(buffer, sizeof(char), numbytes, infile);
      fclose(infile);

      return buffer;
}