#include <asm-generic/errno-base.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../lib/include/logger.h"
#include "../lib/include/picohttpparser.h"

#define PORT 8080
#define LISTEN_BACKLOG 50
#define DEFAULT_LOG_FILEPATH "./logs/log.txt"
#define BUFFER_SIZE 8192
#define SERVER_PORT 80

logger_t *logger;

void *handle_client(void *arg) {
  int client_fd = *(int *)arg;
  free(arg);

  /* Read header */
  char request_buffer[BUFFER_SIZE];
  const char *method, *path;
  int pret, minor_version;
  struct phr_header headers[100];
  size_t buflen = 0, prevbuflen = 0, method_len, path_len, num_headers;
  ssize_t rret;

  while (1) {
    /* Read the request */
    while ((rret = read(client_fd, request_buffer + buflen,
                        sizeof(request_buffer) - buflen)) == -1 &&
           errno == EINTR)
      ;
    if (rret <= 0) {
      logger_log(logger, ERROR, "Error in read from socket/connection closed");
      close(client_fd);
      return NULL;
    }

    prevbuflen = buflen;
    buflen += rret;
    /* Parse the request */
    num_headers = sizeof(headers) / sizeof(headers[0]);
    pret = phr_parse_request(request_buffer, buflen, &method, &method_len,
                             &path, &path_len, &minor_version, headers,
                             &num_headers, prevbuflen);
    if (pret > 0) {
      break; /* Successfully parsed the request */
    } else if (pret == -1) {
      logger_log(logger, ERROR, "Error in parsing header");
      close(client_fd);
      return NULL;
    } else if (pret != -2) {
      logger_log(logger, ERROR, "Unknown error in parsing header");
      close(client_fd);
      return NULL;
    }
    /* Request is incomplete, continue the loop */
    if (buflen == sizeof(request_buffer)) {
      logger_log(logger, ERROR, "Request too long");
      close(client_fd);
      return NULL;
    }
  }

  /* Check method, if not GET, then skip */
  if (method_len != 3 || strncmp(method, "GET", 3) != 0) {
    char *response = "HTTP/1.0 405 Bad Request\r\n\r\n";
    send(client_fd, response, strlen(response), 0);
    logger_log(logger, INFO, "Only GET method supported and get: %s", method);
    close(client_fd);
    return NULL;
  }

  /* Extract host */
  const char *host = NULL;
  size_t host_len = 0;
  for (size_t i = 0; i < num_headers; i++) {
    if (headers[i].name_len == 4 &&
        strncasecmp(headers[i].name, "host", 4) == 0) {
      host = headers[i].value;
      host_len = headers[i].value_len;
      break;
    }
  }

  char host_str[PAGE / 4];
  if (host) {
    snprintf(host_str, sizeof(host_str), "%.*s", (int)host_len, host);
  }

  struct addrinfo hints, *result = NULL;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  int err = getaddrinfo(host_str, NULL, &hints, &result);
  if (err != 0) {
    logger_log(logger, ERROR, "Error in getaddrinfo: %s", gai_strerror(err));
    close(client_fd);
    return NULL;
  }

  /* Connect to host */
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    freeaddrinfo(result);
    char *responce = "HTTP/1.0 502 Bad Gateway\r\n"
                     "Connection: close\r\n\r\n";
    send(client_fd, responce, strlen(responce), 0);
    logger_log(logger, ERROR, "Error in socket creation: %s", strerror(errno));
    close(client_fd);
    return NULL;
  }

  struct sockaddr_in server_addr;
  memcpy(&server_addr, result->ai_addr, result->ai_addrlen);
  server_addr.sin_port = htons(SERVER_PORT);
  freeaddrinfo(result);

  if (connect(server_fd, (struct sockaddr *)&server_addr,
              sizeof(server_addr)) == -1) {
    char *responce = "HTTP/1.0 502 Bad Gateway\r\n"
                     "Connection: close\r\n\r\n";
    send(client_fd, responce, strlen(responce), 0);
    logger_log(logger, ERROR, "Error in connection to server: %s",
               strerror(errno));
    close(server_fd);
    close(client_fd);
    return NULL;
  }

  char path_str[PAGE / 2];
  snprintf(path_str, sizeof(path_str), "%.*s", (int)path_len, path);

  char forward_request[BUFFER_SIZE];
  int forward_len = snprintf(forward_request, sizeof(forward_request),
                             "GET %s HTTP/1.0\r\n"
                             "Host: %s\r\n"
                             "Connection: close\r\n"
                             "\r\n",
                             path_str, host_str);

  if (send(server_fd, forward_request, (size_t)forward_len, 0) == -1) {
    char *responce = "HTTP/1.0 502 Bad Gateway\r\nConnection: close\r\n\r\n";
    send(client_fd, responce, strlen(responce), 0);
    logger_log(logger, ERROR, "Error in send to server: %s", strerror(errno));
    close(server_fd);
    close(client_fd);
    return NULL;
  }

  /* Receive server responce */
  char responce_buffer[BUFFER_SIZE];
  while (1) {
    ssize_t bytes =
        recv(server_fd, responce_buffer, sizeof(responce_buffer), 0);

    if (bytes == 0) {
      break;
    }

    if (bytes < 0) {
      logger_log(logger, ERROR, "Error in recv from server: %s",
                 strerror(errno));
      break;
    }

    if (send(client_fd, responce_buffer, (size_t)bytes, 0) == -1) {
      logger_log(logger, ERROR, "Error in send to client: %s", strerror(errno));
      break;
    }
  }

  close(server_fd);
  close(client_fd);

  return NULL;
}

int main() {

  int err;

  logger = logger_init(DEFAULT_LOG_FILEPATH);
  if (!logger) {
    perror("Can't init logger");
    return 1;
  }

  int server_fd;
  struct sockaddr_in server_addr;

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    logger_log(logger, FATAL, "Error in creating socket: %s", strerror(errno));
    logger_destroy(logger);
    return 1;
  }

  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  err = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (err == -1) {
    logger_log(logger, FATAL, "Error in binding server socket");
    logger_destroy(logger);
    close(server_fd);
    return 1;
  }

  err = listen(server_fd, LISTEN_BACKLOG);
  if (err == -1) {
    logger_log(logger, FATAL, "Error in listening server socket");
    logger_destroy(logger);
    close(server_fd);
    return 1;
  }

  logger_log(logger, INFO, "HTTP Proxy started. Listening on port: %d", PORT);

  while (1) {
    int *client_fd;
    socklen_t client_addr_size;
    struct sockaddr_in client_addr;

    client_addr_size = sizeof(client_addr);

    client_fd = (int *)malloc(sizeof(int));
    *client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size);
    if (*client_fd == -1) {
      logger_log(logger, ERROR, "Client accept failed");
      continue;
    }

    logger_log(logger, INFO, "Accept client, fd = %d", *client_fd);

    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    err = pthread_create(&tid, &attr, handle_client, client_fd);
    if (err != 0) {
      logger_log(logger, ERROR, "Error in pthread_create: %s", strerror(err));
      close(*client_fd);
      continue;
    }

    pthread_attr_destroy(&attr);
  }

  // todo: graceful shutdown
  pthread_exit(0);

  logger_destroy(logger);

  close(server_fd);

  return 0;
}
