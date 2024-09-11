#ifndef SSL_UTILS_H
#define SSL_UTILS_H

#include <openssl/ssl.h>

void initialize_openssl();
void cleanup_openssl();
SSL_CTX *create_ssl_context();

#endif 
