#include "network.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// create and connect a socket to the server
int create_socket(const char *hostname, int port) {
  int sock;
  struct sockaddr_in server_addr;
  struct hostent *server;

  // create a socket
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Error creating socket");
    return -1;
  }

  // resolve the hostname to an ip address
  server = gethostbyname(hostname);
  if (server == NULL) {
    fprintf(stderr, "Error: No such host found.\n");
    close(sock);
    return -1;
  }

  // set up the server address struct
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

  // connect to the server
  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("Error connecting to server");
    close(sock);
    return -1;
  }

  return sock;
}

// send an https request GET request
void send_https_request(SSL *ssl, const char *hostname, const char *path) {
  char request[1024];

  snprintf(request, sizeof(request),
           "GET %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Connection: close\r\n\r\n",
           path, hostname);

  SSL_write(ssl, request, strlen(request));
}
