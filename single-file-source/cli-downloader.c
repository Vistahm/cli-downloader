#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

// initialize openssl library
void initialize_openssl() {
  SSL_load_error_strings();
  OpenSSL_add_ssl_algorithms();
}

// cleanup openssl library
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

// signal handler for Ctrl+C (SIGINT) to terminate download with a message
void handle_sigint(int signum) {
  printf("\nDownload terminated.\n");
  exit(0);
}

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

  // resolve the hostname to an IP address
  server = gethostbyname(hostname);
  if (server == NULL) {
    fprintf(stderr, "Error: No such host found.\n");
    close(sock);
    return -1;
  }

  // Set up the server address struct
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

  // Connect to the server
  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("Error connecting to server");
    close(sock);
    return -1;
  }

  return sock;
}

// Function to send an HTTPS GET request
void send_https_request(SSL *ssl, const char *hostname, const char *path) {
  char request[1024];

  snprintf(request, sizeof(request),
           "GET %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Connection: close\r\n\r\n",
           path, hostname);

  SSL_write(ssl, request, strlen(request));
}

// Convert bytes to a human-readable format (KB, MB, etc.)
void format_size(char *output, long bytes) {
  if (bytes < 1024) {
    sprintf(output, "%ldB", bytes);
  } else if (bytes < 1024 * 1024) {
    sprintf(output, "%.1fKB", (double)bytes / 1024);
  } else if (bytes < 1024 * 1024 * 1024) {
    sprintf(output, "%.1fMB", (double)bytes / (1024 * 1024));
  } else {
    sprintf(output, "%.1fGB", (double)bytes / (1024 * 1024 * 1024));
  }
}

// Function to calculate elapsed time in seconds
double get_elapsed_time(struct timeval start, struct timeval end) {
  return (double)(end.tv_sec - start.tv_sec) +
         (double)(end.tv_usec - start.tv_usec) / 1000000.0;
}

// Function to display the download progress bar
void display_progress_bar(long downloaded, long total, double speed) {
  int bar_width = 30;
  float progress = (float)downloaded / total;
  int pos = (int)(bar_width * progress);

  char total_size_str[20], downloaded_size_str[20];
  format_size(total_size_str, total);
  format_size(downloaded_size_str, downloaded);

  char speed_str[20];
  format_size(speed_str, (long)speed);

  printf("[");
  for (int i = 0; i < bar_width; ++i) {
    if (i < pos)
      printf("#");
    else if (i == pos)
      printf(">");
    else
      printf("-");
  }
  printf("] %d%% %s %s %s/s\r", (int)(progress * 100), total_size_str,
         downloaded_size_str, speed_str);
  fflush(stdout);
}

// Function to download the file and save it locally using HTTPS with progress
// bar and speed calculation
void download_file_https(SSL *ssl, const char *output_path) {
  FILE *file = fopen(output_path, "wb");
  if (file == NULL) {
    perror("Error opening output file");
    return;
  }

  char buffer[BUFFER_SIZE];
  int bytes_received;
  int header_parsed = 0;
  char *body_start = NULL;
  long content_length = 0;
  long total_downloaded = 0;
  struct timeval start_time, current_time;

  gettimeofday(&start_time, NULL); 

  while ((bytes_received = SSL_read(ssl, buffer, BUFFER_SIZE)) > 0) {
    // look for the end of the HTTPs header
    if (!header_parsed) {
      buffer[bytes_received] = '\0';
      body_start = strstr(buffer, "\r\n\r\n");
      if (body_start) {
        body_start += 4; // move past the "\r\n\r\n"
        header_parsed = 1;

        // Look for "Content-Length" in the header
        char *content_length_str = strstr(buffer, "Content-Length: ");
        if (content_length_str) {
          sscanf(content_length_str, "Content-Length: %ld", &content_length);
        }

        // Write any body content that was already received
        size_t header_size = body_start - buffer;
        fwrite(body_start, 1, bytes_received - header_size, file);
        total_downloaded += bytes_received - header_size;

        // Display initial progress bar
        if (content_length > 0) {
          gettimeofday(&current_time, NULL); // Get the current time
          double elapsed_time = get_elapsed_time(start_time, current_time);
          double speed = total_downloaded / elapsed_time;
          display_progress_bar(total_downloaded, content_length, speed);
        }
      }
    } else {
      // Write body content directly after header has been parsed
      fwrite(buffer, 1, bytes_received, file);
      total_downloaded += bytes_received;

      // Display updated progress bar
      if (content_length > 0) {
        gettimeofday(&current_time, NULL); // Get the current time
        double elapsed_time = get_elapsed_time(start_time, current_time);
        double speed = total_downloaded / elapsed_time;
        display_progress_bar(total_downloaded, content_length, speed);
      }
    }
  }

  if (bytes_received < 0) {
    perror("Error receiving data");
  }

  printf("\n");
  fclose(file);
}

// Function to extract file name from URL
char *get_filename_from_url(const char *url) {
  const char *last_slash = strrchr(url, '/');
  if (last_slash == NULL || *(last_slash + 1) == '\0') {
    // No slash found or nothing after the last slash, default to
    // "downloaded_file"
    return "downloaded_file";
  }
  return (char *)(last_slash + 1);
}

int main(int argc, char *argv[]) {
  if (argc < 2 || argc > 3) {
    fprintf(stderr, "Usage: %s <URL> [filename]\n", argv[0]);
    return 1;
  }

  char *url = argv[1];
  char *filename = (argc == 3) ? argv[2] : get_filename_from_url(url);

  // Extract the hostname and path from the URL
  char hostname[256];
  char path[256];
  int port = 443; // Default HTTPS port

  sscanf(url, "https://%255[^/]%255[^\n]", hostname, path);

  printf("Connecting to the provided server...\n");

  // Set up signal handler
  signal(SIGINT, handle_sigint);

  initialize_openssl();
  SSL_CTX *ctx = create_ssl_context();

  // Create and connect socket
  int sock = create_socket(hostname, port);
  if (sock < 0) {
    SSL_CTX_free(ctx);
    cleanup_openssl();
    return 1;
  }

  // Create SSL structure and establish SSL connection
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

  // Send HTTPS GET request
  send_https_request(ssl, hostname, path);

  // Download file using HTTPS
  download_file_https(ssl, filename);

  // Clean up
  SSL_shutdown(ssl);
  SSL_free(ssl);
  close(sock);
  SSL_CTX_free(ctx);
  cleanup_openssl();

  printf("Download complete. Saved as %s\n", filename);
  return 0;
}
