#include "file_utils.h"
#include "progress.h"
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define BUFFER_SIZE 4096

// download the file and save it locally using https with progress bar and speed calculation
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
    // look for the end of the https header
    if (!header_parsed) {
      buffer[bytes_received] = '\0';
      body_start = strstr(buffer, "\r\n\r\n");
      if (body_start) {
        body_start += 4;  // move past the '\r\n\r\n'
        header_parsed = 1;
        
        // look for "Content-Length" in the header
        char *content_length_str = strstr(buffer, "Content-Length: ");
        if (content_length_str) {
          sscanf(content_length_str, "Content-Length: %ld", &content_length);
        }

        // write any body content that was already received
        size_t header_size = body_start - buffer;
        fwrite(body_start, 1, bytes_received - header_size, file);
        total_downloaded += bytes_received - header_size;

        // display initial progress bar
        if (content_length > 0) {
          gettimeofday(&current_time, NULL);  // get the current time
          double elapsed_time = get_elapsed_time(start_time, current_time);
          double speed = total_downloaded / elapsed_time;
          display_progress_bar(total_downloaded, content_length, speed);
        }
      }
    } else {
      // write body content directly after header has been parsed
      fwrite(buffer, 1, bytes_received, file);
      total_downloaded += bytes_received;

      // display updated progress bar
      if (content_length > 0) {
        gettimeofday(&current_time, NULL);
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

// extract file name from url
char *get_filename_from_url(const char *url) {
  const char *last_slash = strrchr(url, '/');
  if (last_slash == NULL || *(last_slash + 1) == '\0') {
    // if now slash found or nothing after the last slash,
    // choose a default name
    return "downloaded_file";
  }
  return (char *)(last_slash + 1);
}
