#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 80
#define LISTEN_BACKLOG 50

void *handle_client(void *arg) {
  int client_fd = *(int *)arg;

  return NULL;
}

int main() {

  int err;

  int server_fd;
  struct sockaddr_in server_addr;

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    perror("Error in creating a socket");
    return 1;
  }

  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  err = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (err == -1) {
    perror("Error in binding server socket");
    return 1;
  }

  err = listen(server_fd, LISTEN_BACKLOG);
  if (err == -1) {
    perror("Error in listening server socket");
    return 1;
  }

  while (1) {
    int *client_fd;
    socklen_t client_addr_size;
    struct sockaddr_in client_addr;

    client_addr_size = sizeof(client_addr);

    client_fd = (int *)malloc(sizeof(client_fd));
    *client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_size);
    if (*client_fd == -1) {
      perror("Error in accept");
      continue;
    }

    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    err = pthread_create(&tid, &attr, handle_client, client_fd);
    if (err != 0) {
      fprintf(stderr, "Error in pthread_create for %lu: %s", tid,
              strerror(err));
      close(*client_fd);
      continue;
    }
  }

  pthread_exit(0);

  close(server_fd);

  return 0;
}
