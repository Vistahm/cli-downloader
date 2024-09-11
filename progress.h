#ifndef PROGRESS_H
#define PROGRESS_H

#include <sys/time.h>

void display_progress_bar(long downloaded, long total, double speed);
void format_size(char *output, long bytes);
double get_elapsed_time(struct timeval start, struct timeval end);

#endif 
