#include "file_utils.h"
#include "network.h"
#include "ssl_utils.h"
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// signal handler for ctrl + c (SIGINT) to terminate download with a message
void handle_sigint(int signum) {
  printf("\nDownload terminated.\n");
  exit(0);
}

int main(int argc, char *argv[]) {
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage: %s <URL> [filename]\n", argv[0]);
    return 1;
  }

  char *url = argv[1];
  char *filename = (argc == 3) ? argv[2] : get_filename_from_url(url);

  // extract the hostname and path from the URL
  char hostname[256];
  char path[256];
  int port = 443;  // default port for https

  sscanf(url, "https://%255[^/]%255[^\n]", hostname, path);

  printf("Connecting to the provided server...\n");

  // set up signal handler
  signal(SIGINT, handle_sigint);

  initialize_openssl();
  SSL_CTX *ctx = create_ssl_context();

  // create and connect socket
  int sock = create_socket(hostname, port);
  if (sock < 0) {
    SSL_CTX_free(ctx);
    cleanup_openssl();
    return 1;
  }

  // create SSL structure and establish SSL connection
  SSL *ssl = SSL_new(ctx);
  SSL_set_fd(ssl, sock);

  if (SSL_connect(ssl) <= 0) {
    ERR_print_errors_fp(stderr);
    close(sock);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    cleanup_openssl();
    return 1;
  }

  // send https GET request
  send_https_request(ssl, hostname, path);

  // download file using https
  download_file_https(ssl, filename);

  // clean up
  SSL_shutdown(ssl);
  SSL_free(ssl);
  close(sock);
  SSL_CTX_free(ctx);
  cleanup_openssl();

  printf("Download complete. Saved as %s\n", filename);
  return 0;
}
