#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_PORT 12345
#define MAXLINE 4096
#define SOCKET_ERROR (-1)

/*
 * Exit codes:
 * 1 - Failed to create socket
 * 7 - Failed to bind socket
 * 8 - Failed to listen
 */

void error(const char *err_msg) {
  int errno_save = errno;
  fprintf(stderr, "Error: %s \n"  // err_msg
                  "(%d: "       // errno_save
                  "%s)\n",        // strerror(errno_save)
          err_msg, errno_save, strerror(errno_save));
  fflush(stderr);
}

int main() {
  int serv_sockfd = SOCKET_ERROR, client_sockfd = SOCKET_ERROR, port = SERVER_PORT;
  ssize_t n;
  socklen_t client_len;
  char buffer[MAXLINE];
  struct sockaddr_in serv_addr, client_addr;

  serv_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (serv_sockfd < 0) {
    error("Failed to create socket");
    exit(1);
  }

  memset((char *)&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);

  if (bind(serv_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    error("Failed to bind socket");
    close(serv_sockfd);
    exit(7);
  }

  if (listen(serv_sockfd, SOMAXCONN) == SOCKET_ERROR) {
    error("Failed to listen");
    close(serv_sockfd);
    exit(8);
  }

  puts("Server is running...");
  for (;;) {
    client_len = sizeof(client_addr);
    client_sockfd = accept(serv_sockfd, (struct sockaddr *) &client_addr, &client_len);
    if (client_sockfd == SOCKET_ERROR) {
      error("Failed to accept client connection");
      break;
    }

    memset(buffer, 0, sizeof(buffer));
    n = recv(client_sockfd, buffer, sizeof(buffer) - 1, 0);
    if (n > 0) {
      buffer[n] = '\0';
    } else if (n == 0) {
      error("Client close the connection");
      close(client_sockfd);
      break;
    }

    printf("Get: %s\n", buffer);

    char *answer = "Hello from the server!";
    if ((send(client_sockfd, answer, strlen(answer), 0)) == SOCKET_ERROR) {
      error("Failed to send response to client");
      close(client_sockfd);
      break;
    }
    close(client_sockfd);
  }

  close(serv_sockfd);

  return 0;
}
