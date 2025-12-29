#ifndef LOGGER_H
#define LOGGER_H

#include <pthread.h>
#include <stdio.h>

#define GREEN "\033[92m"
#define LIGHT_BLUE "\033[36m"
#define BLUE "\033[34m"
#define RED "\033[91m"
#define BCK_RED "\033[101m"
#define NO_COLOR "\x1b[0m"

#define PAGE 4096

enum log_level { DEBUG, INFO, ERROR, FATAL };

typedef struct logger {
  int log_file_fd;

  pthread_mutex_t log_mutex;

} logger_t;

logger_t * logger_init(char *log_filepath);

int logger_log(logger_t *logger, enum log_level level, const char *fmt, ...);

void logger_destroy(logger_t *logger);

#endif // LOGGER_H