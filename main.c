#define _DEFAULT_SOURCE

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  char *srvr_addr = NULL;
  char *srvr_port = "9000";
  int s;                       // socket file descriptor
  struct sockaddr_in adr_srv;  // socket to contact the server
  struct sockaddr_in adr_clnt; // socket to manage client request
  int z, client_connection;    // contains return values to check for errors
  int addr_len;
  time_t rawtime;
  char tt[100];
  char buffer[10000];

  // get server ip address from command line
  if (argc >= 2) {
    srvr_addr = argv[1];
  } else {
    srvr_addr = "127.0.0.1";
  }

  if (argc >= 3) {
    srvr_port = argv[2];
  }

  printf("Server address: %s\n", srvr_addr);
  printf("Server port: %s\n", srvr_port);

  s = socket(PF_INET, SOCK_STREAM, 0);
  if (s == -1) {
    perror("socket creation error");
    exit(EXIT_FAILURE);
  }
  memset(&adr_srv, 0, sizeof(adr_srv)); // clear sockaddr_in structure
  adr_srv.sin_family = AF_INET;
  adr_srv.sin_port = htons(atoi(srvr_port));
  inet_pton(AF_INET, srvr_addr, &adr_srv.sin_addr);
  if (adr_srv.sin_addr.s_addr == INADDR_NONE) {
    perror("no server address");
    exit(EXIT_FAILURE);
  }

  // bind return value in z: 0 if everything is ok, -1 on error
  z = bind(s, (struct sockaddr *)&adr_srv, sizeof(adr_srv));
  if (z == -1) {
    perror("bind error");
    exit(EXIT_FAILURE);
  }

  z = listen(s, 10);
  if (z == -1) {
    perror("listen() error");
    exit(EXIT_FAILURE);
  }
  puts("\nStart Listening");

  for (;;) {

    addr_len = sizeof adr_clnt;
    client_connection =
        accept(s, (struct sockaddr *)&adr_clnt, (socklen_t *)&addr_len);
    if (client_connection == -1) {
      perror("accepting connection error");
      continue;
    }

    memset(buffer, 0, sizeof buffer);
    int num = 0;
    while (1) {
      if ((num = recv(client_connection, buffer, sizeof(buffer), 0)) > 0) {
        // data in buffer
        printf("\n------------------\n");
        time(&rawtime);
        printf("request arrived at: %s\n", ctime(&rawtime));
        printf("%s\n", buffer);

        // Behaviour to implement
        // - check if the message has a body looking for "Content-Length:"
        // header,
        // - if there is a body, understand where it starts
        // - read Content-Length bytes
        // - generate a response

        int i;
        char *enventual_body_starts_here;
        for (i = 0; i < strlen(buffer); i++) {
          if (buffer[i] == '\r' && buffer[i + 1] == '\n' &&
              buffer[i + 2] == '\r' && buffer[i + 3] == '\n') {

            enventual_body_starts_here = (char *)(buffer + i + 4);
          }
        }

        char *clp;
        int content_length = 0;
        clp = strstr(buffer, "Content-Length:");
        if (clp == NULL) {
          // no content
          break;
        }

        content_length = atoi(clp + 15);

        if (strlen(enventual_body_starts_here) < content_length) {
          printf("we should read the last part of the message - not "
                 "implemented - program is exiting");
          exit(EXIT_FAILURE);
        }

        // ok I have the entire message, I can proceed to respond
        break;

      } else if (num == 0) {
        perror("connection closed by client");
        // what should I do ?
        close(client_connection);
        continue;

      } else {
        perror("error while reading request");
        // what should I do ?
        close(client_connection);
        continue;
      }
    }

    sprintf(tt, "HTTP/1.1 200 OK\nContent-Type: "
                "application/json\n\n{\"ciao\":\"pippo\"}\n\n");
    time(&rawtime);
    z = write(client_connection, tt, strlen(tt));
    if (z == -1) {
      perror("write() error");
      exit(EXIT_FAILURE);
    }
    printf("response sent at: %s\n", ctime(&rawtime));
    printf("%s\n", tt);
    close(client_connection);
  }

  close(s);
  return 0;
}
