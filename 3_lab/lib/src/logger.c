#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../include/logger.h"

void show_logo(void);

logger_t *logger_init(char *log_filepath) {
  logger_t *logger = (logger_t *)malloc(sizeof(logger_t));

  if (!logger) {
    return NULL;
  }

  if (log_filepath) {
    logger->log_file_fd =
        open(log_filepath, O_APPEND | O_CREAT | O_WRONLY, 0644);

    if (logger->log_file_fd == -1) {
      free(logger);
      return NULL;
    }
  } else {
    logger->log_file_fd = -1;
  }

  if (pthread_mutex_init(&logger->log_mutex, NULL) != 0) {
    if (logger->log_file_fd != -1) {
      close(logger->log_file_fd);
    }

    free(logger);
    return NULL;
  }

  show_logo();

  return logger;
}

char *log_level_color(enum log_level level) {
  switch (level) {
  case DEBUG:
    return GREEN;
  case INFO:
    return BLUE;
  case ERROR:
    return RED;
  case FATAL:
    return BCK_RED;
  default:
    return NO_COLOR;
  }
}

char *log_level_str(enum log_level level) {
  switch (level) {
  case DEBUG:
    return "DEBUG";
  case INFO:
    return "INFO";
  case ERROR:
    return "ERROR";
  case FATAL:
    return "FATAL";
  default:
    return "UNKNOWN";
  }
}

int logger_log(logger_t *logger, enum log_level level, const char *fmt, ...) {
  if (!logger || !fmt)
    return 1;

  time_t now = time(NULL);
  struct tm tm;
  localtime_r(&now, &tm);

  char ts[64];
  if (strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tm) == 0)
    return 1;

  char msgbuf[PAGE / 2];

  va_list ap;
  va_start(ap, fmt);
  int mn = vsnprintf(msgbuf, sizeof(msgbuf), fmt, ap);
  va_end(ap);

  if (mn < 0)
    return 1;

  char file_line[PAGE / 2];
  int fn = snprintf(file_line, sizeof(file_line), "%s [%s] %s\n", ts,
                    log_level_str(level), msgbuf);
  if (fn < 0)
    return 1;

  char out_line[PAGE / 2];
  int on = snprintf(out_line, sizeof(out_line), "%s%s [%s] %s%s\n",
                    log_level_color(level), ts, log_level_str(level), msgbuf,
                    NO_COLOR);
  if (on < 0)
    return 1;

  size_t file_len = strnlen(file_line, sizeof(file_line));
  size_t out_len = strnlen(out_line, sizeof(out_line));

  if (pthread_mutex_lock(&logger->log_mutex) != 0)
    return 1;

  if (logger->log_file_fd != -1) {
    if (write(logger->log_file_fd, file_line, file_len) == -1)
      return 1;
  }

  if (write(STDOUT_FILENO, out_line, out_len) == -1)
    return 1;

  pthread_mutex_unlock(&logger->log_mutex);
  return 0;
}

void logger_destroy(logger_t *logger) {
  if (!logger) {
    return;
  }

  pthread_mutex_destroy(&logger->log_mutex);

  if (logger->log_file_fd != -1) {
    close(logger->log_file_fd);
  }

  free(logger);
}

void show_logo(void) {
  FILE *fp = fopen("./others/logo.txt", "r");
  if (!fp) {
    perror("logo.txt");
    return;
  }

  char line[256];

  printf("%s", GREEN);
  fflush(stdout);

  while (fgets(line, sizeof(line), fp)) {
    for (int i = 0; line[i] != '\0'; i++) {
      putchar(line[i]);
      fflush(stdout);
      usleep(50);
    }
  }

  printf("%s\n", NO_COLOR);
  fflush(stdout);

  fclose(fp);
}
