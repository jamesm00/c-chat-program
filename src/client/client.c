#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

int main(int argc, char* argv[]) {
  int status = 0;
  struct sockaddr_in server_address;
  char write_buffer[1024];
  char receive_buffer[1024];
  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_fd < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(PORT);
  server_address.sin_addr.s_addr = INADDR_ANY;

  if ((status = connect(socket_fd, 
    (struct sockaddr *) &server_address, sizeof(server_address))) < 0) {
    perror("Could not connect to the server");
    exit(EXIT_FAILURE);
  }

  while (1) {
    fgets(write_buffer, 1024, stdin);
    write(socket_fd, write_buffer, strlen(write_buffer) + 1);
    read(socket_fd, receive_buffer, 1023);
    printf("Message from the server: %s\n", receive_buffer);
  }
  
  close(socket_fd);

  return 0;
}