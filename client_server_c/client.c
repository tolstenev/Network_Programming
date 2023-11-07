#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>

#define SERVER_PORT 12345
#define MAXLINE 4096

/*
 * Exit codes:
 * 1 - Failed to create socket
 * 2 - Invalid address or address not supported
 * 3 - The host does not exist
 * 4 - Connection establishment error
 * 5 - Error sending a message
 * 6 - Error receiving a message
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
  int sockfd;
  ssize_t n;
  struct hostent *server;
  char buffer[MAXLINE];

  struct sockaddr_in serv_addr;
  char *serv_ip = "127.0.0.1";
  int serv_port = SERVER_PORT;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    error("Failed to create socket");
    exit(1);
  }

  memset((char *)&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(serv_port);
  if (inet_pton(AF_INET, serv_ip, &(serv_addr.sin_addr)) <= 0) {
    error("The invalid address or address not supported");
    close(sockfd);
    exit(2);
  }

  server = gethostbyaddr((char *) &serv_addr.sin_addr, 4, AF_INET);
  if (server == NULL) {
    error("The host does not exist");
    close(sockfd);
    exit(3);
  }

  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    error("Connection establishment error");
    close(sockfd);
    exit(4);
  }

  memset(buffer, 0, sizeof(buffer));
  strcpy(buffer, "Message from client!");
  if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
    error("Error sending the message");
    close(sockfd);
    exit(5);
  }

  memset(buffer, 0, sizeof(buffer));

  char string[MAXLINE] = {0};
  for (;;) {
    n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (n < 0) {
      error("Error receiving a message");
      close(sockfd);
      exit(6);
    } else if (n == 0) {
      break;
    }
    strcat(string, buffer);
    memset(buffer, 0, sizeof(buffer));
  }

  printf("Get: %s\n", string);

  close(sockfd);

  return 0;
}
