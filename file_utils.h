#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <openssl/crypto.h>

void download_file_https(SSL *ssl, const char *output_path);
char *get_filename_from_url(const char *url);

#endif
