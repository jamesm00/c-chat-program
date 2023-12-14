#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

pthread_t reader_thread, writer_thread;
int socket_fd;

void* reader(void* args) {
    char receive_buffer[1024];
    while (1) {
        if (!read(socket_fd, receive_buffer, 1024)) {
            break;
        }
        printf("%s\n", receive_buffer);
    }
    pthread_exit(NULL);
}

void* writer(void* args) {
    char write_buffer[1000];
    while (1) {
        fgets(write_buffer, 1000, stdin);
        write(socket_fd, write_buffer, strlen(write_buffer) + 1);
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    int status = 0;
    struct sockaddr_in server_address;
    // Username with maximum of 20 characters + null character.
    char username[21];

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
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

    printf("Successfully connected to the server.\n\n");

    // Prompt for the username and write it to the server.
    printf("Enter your username: ");
    fgets(username, 21, stdin);
    size_t last = strlen(username) - 1;
    if (username[last] == '\n')
        username[last] = '\0';
    write(socket_fd, username, strlen(username) + 1);

    pthread_create(&reader_thread, NULL, reader, NULL);
    pthread_create(&writer_thread, NULL, writer, NULL);

    pthread_join(reader_thread, NULL);
    pthread_join(writer_thread, NULL);

    close(socket_fd);

    return 0;
}