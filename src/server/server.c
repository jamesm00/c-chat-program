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
#define MAX_TEXT_SIZE 1000
#define MAX_USERNAME_SIZE 20
#define MAX_RESPONSE_SIZE MAX_USERNAME_SIZE + MAX_TEXT_SIZE + 10

struct client {
    pthread_t thread;
    int socket_fd;
    char name[MAX_USERNAME_SIZE];
};

struct client clients[MAX_CLIENT_COUNT];
int client_count = 0;

void* handle_client(void* args) {
    char buffer[MAX_TEXT_SIZE] = {0};
    char response[MAX_RESPONSE_SIZE];
    int bytes_read = 0;
    struct client* client_info = (struct client *) args;
    int socket_fd = client_info->socket_fd;

    // client joined
    // Read client username.
    bytes_read = read(socket_fd, buffer, 21);
    printf("%s has connected to the server.\n", buffer);
    strncpy(client_info->name, buffer, 21);

    while (1) {
        if ((bytes_read = read(socket_fd, buffer, MAX_TEXT_SIZE)) == 0) {
            break;
        }

        printf("From client %s: %s\n", client_info->name, buffer);
        snprintf(response, MAX_RESPONSE_SIZE, "%s: %s", client_info->name, buffer);

        // Either concatenate the strings to make the write() atomic,
        // or use a lock to call write() twice in a critical section.

        for (int i = 0; i < client_count; i++) { 
            write(clients[i].socket_fd, response, strlen(response) + 1);
        }
    }

    // client disconnected
    printf("%s has disconnected from the server.\n", client_info->name);

    close(socket_fd);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int opt = 1;
    ssize_t valread;

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

    if (listen(server_fd, MAX_CLIENT_COUNT) < 0) {
        perror("Failed to listen for connections");
        exit(EXIT_FAILURE);
    }

    while (client_count < MAX_CLIENT_COUNT) {
        if ((clients[client_count].socket_fd 
            = accept(server_fd, (struct sockaddr *) &address, &addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        pthread_create(&clients[client_count].thread, NULL, handle_client, 
            (void *) &clients[client_count]);
        client_count++;
    }

    for (int i = 0; i < client_count; i++) {
        pthread_join(clients[i].thread, NULL);
    }

    close(server_fd);

    return 0;
}