#define _DEFAULT_SOURCE

#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_LENGTH 10000

int is_valid_number(const char *str) {
  int i;
  for (i = 0; i < strlen(str); i++) {
    if (str[i] <= '0' || str[i] >= '9') {
      return 0;
    }
  }
  return 1;
}

int is_valid_uuid(const char *str) {
  if (str == NULL)
    return 0;
  if (strlen(str) != 36)
    return 0;

  for (int i = 0; i < 36; i++) {
    if (i == 8 || i == 13 || i == 18 || i == 23) {
      if (str[i] != '-')
        return 0;
    } else {
      if (!isxdigit(str[i]))
        return 0;
    }
  }
  return 1;
}

int main(int argc, char *argv[]) {
  char *srvr_addr = NULL;
  char *srvr_port = "9999";
  int s;                       // socket file descriptor
  struct sockaddr_in adr_srv;  // socket to contact the server
  struct sockaddr_in adr_clnt; // socket to manage client request
  int z, client_connection;    // contains return values to check for errors
  int addr_len;
  time_t rawtime;
  char tt[100];
  char buffer[BUFFER_LENGTH];
  char response_buffer[BUFFER_LENGTH];

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

  char request_target[100] = {0};
  char request_method[10] = {0};
  for (;;) {

    memset(response_buffer, 0, BUFFER_LENGTH);
    addr_len = sizeof adr_clnt;
    client_connection =
        accept(s, (struct sockaddr *)&adr_clnt, (socklen_t *)&addr_len);
    if (client_connection == -1) {
      perror("accepting connection error");
      continue;
    }

    memset(buffer, 0, sizeof buffer);
    int num = 0;
    int i, j;
    while (1) {
      if ((num = recv(client_connection, buffer, sizeof(buffer), 0)) > 0) {
        // data in buffer
        printf("\n------------------\n");
        time(&rawtime);
        printf("request arrived at: %s\n", ctime(&rawtime));
        printf("%s\n", buffer);

        // Behaviour to implement
        // - understand where headers ends (and body starts)
        // - check if the message has a body looking for "Content-Length:"
        // header,
        // - extract "request-target" from request
        // - read Content-Length bytes
        // - generate a response

        char *enventual_body_starts_here;
        for (i = 0; i < strlen(buffer); i++) {
          if (buffer[i] == '\r' && buffer[i + 1] == '\n' &&
              buffer[i + 2] == '\r' && buffer[i + 3] == '\n') {

            enventual_body_starts_here = (char *)(buffer + i + 4);
          }
        }

        // extract the request_method from response header
        for (i = 0; i < strlen(buffer); i++) {
          if (buffer[i] == ' ') {
            strlcpy(request_method, buffer, i + 1);
            request_method[i + 1] = '\0';
            break;
          }
        }

        // extract the request_target from response header
        for (i = 0; i < strlen(buffer); i++) {
          if (buffer[i] == '\r' && buffer[i + 1] == '\n') {
            int start = 0, end = 0;
            for (j = 0; j < i; j++) {
              if (buffer[j] == '/' && start == 0) {
                start = j;
              } else if ((buffer[j] == ' ' || buffer[j] == '?') && start != 0) {
                end = j;
              }
              if (start != 0 && end != 0) {
                strlcpy(request_target, &(buffer[start]), end - start + 1);
                request_target[end - start + 2] = '\0';
                break;
              }
            }
            break;
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

    // Parse the request_target in order to provide a response
    // The idea is to map the request target to the filesystem
    // an example for: GET /cities/12345/shops
    // the file containing the response has to searched on
    //  ~/mockserver/cities/_/shops/GET

    if (getenv("HOME") == NULL) {
      printf("No HOME variable set. please do it and rerun this program\n");
      exit(EXIT_FAILURE);
    }

    char mapped_file_path[200];
    strcpy(mapped_file_path, getenv("HOME"));
    strcat(mapped_file_path, "/mockserver");

    char request_target_copy[100];
    strcpy(request_target_copy, request_target);
    char *token = strtok(request_target_copy, "/");
    while (token != NULL) {
      // for each token search for the relative directory strarting from
      // ~/mockserver/

      // I should recognize uuid or numbers only strings and convert in "_",
      // as "variable" part in usri path are mapped to filesystem as "_"

      if (is_valid_number(token) == 1 || is_valid_uuid(token) == 1) {
        token[0] = '_';
        token[1] = '\0';
      }

      strcat(mapped_file_path, "/");
      strcat(mapped_file_path, token);

      token = strtok(NULL, "/");
    }
    strcat(mapped_file_path, "/");
    strcat(mapped_file_path, request_method);

    printf("Request-target: %s\n", request_target);
    printf("Request-method: %s\n", request_method);
    printf("mapped_file: %s\n", mapped_file_path);

    int fd = open(mapped_file_path, O_RDONLY);
    if (fd == -1) {
      printf("mapped_file doesn't exists");
      exit(EXIT_FAILURE);
    }

    int nread = read(fd, response_buffer, BUFFER_LENGTH);
    if(nread == -1){
      perror("error reading mapped_file");
      exit(EXIT_FAILURE);

    }

    time(&rawtime);
    z = write(client_connection, response_buffer, strlen(response_buffer));
    if (z == -1) {
      perror("write() error");
      exit(EXIT_FAILURE);
    }

    printf("response sent at: %s\n", ctime(&rawtime));
    printf("%s\n", response_buffer);
    close(client_connection);
  }

  close(s);
  return 0;
}
