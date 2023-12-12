#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define MAX_CLIENT_COUNT 10

pthread_t client_threads[MAX_CLIENT_COUNT];

int client_count = 0;

void* handle_client(void* params) {
  char buffer[1024] = {0};
  int bytes_read = 0;
  int socket_fd = *((int *) params);

  while (1) {
    bytes_read = read(socket_fd, buffer, 1023);
    buffer[bytes_read] = '\0';

    printf("From a client: %s\n", buffer);
    send(socket_fd, buffer, strlen(buffer) + 1, 0);
  }

  close(socket_fd);
  pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
  int server_fd, new_socket;
  struct sockaddr_in address;
  socklen_t addrlen = sizeof(address);
  int opt = 1;
  ssize_t valread;
  int tids[MAX_CLIENT_COUNT];

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Failed to create socket");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt() failed");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
    perror("Failed to bind server socket");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 10) < 0) {
    perror("Failed to listen for connections");
    exit(EXIT_FAILURE);
  }

  while (client_count < MAX_CLIENT_COUNT) {
    if ((tids[client_count] 
      = accept(server_fd, (struct sockaddr *) &address, &addrlen)) < 0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    pthread_create(&client_threads[client_count], NULL, handle_client, 
      (void *) &tids[client_count]);
    client_count++;
  }

  for (int i = 0; i < client_count; i++) {
    pthread_join(client_threads[i], NULL);
  }

  close(server_fd);

  return 0;
}