#ifndef NETWORK_H
#define NETWORK_H

#include <openssl/ssl.h>

int create_socket(const char *hostname, int port);
void send_https_request(SSL *ssl, const char *hostname, const char *path);

#endif 
