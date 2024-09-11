#include "progress.h"
#include <stdio.h>
#include <sys/time.h>

// convert bytes to human-readable formats (KB, MB, GB)
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

// calculate elapsed time in seconds
double get_elapsed_time(struct timeval start, struct timeval end) {
  return (double)(end.tv_sec - start.tv_sec) +
         (double)(end.tv_usec - start.tv_usec) / 1000000.0;
}

// display the download progress bar
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
