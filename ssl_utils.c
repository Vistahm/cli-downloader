#include "ssl_utils.h"
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>

// initialize openssl library
void initialize_openssl() {
  SSL_load_error_strings();
  OpenSSL_add_ssl_algorithms();
}

// clean up openssl library
void cleanup_openssl() { EVP_cleanup(); }

// create SSL context
SSL_CTX *create_ssl_context() {
  const SSL_METHOD *method = SSLv23_client_method();
  SSL_CTX *ctx = SSL_CTX_new(method);
  if (!ctx) {
    perror("Unable to create SSL context");
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }
  return ctx;
}
