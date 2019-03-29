#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#ifndef PORT
  #define PORT 59997
#endif
#define BUF_SIZE 128

int main(void) {
    // Create the socket FD.
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("client: socket");
        exit(1);
    }

    // Set the IP and port of the server to connect to.
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) < 1) {
        perror("client: inet_pton");
        close(sock_fd);
        exit(1);
    }

    // Connect to the server.
    if (connect(sock_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("client: connect");
        close(sock_fd);
        exit(1);
    }

    fd_set all_fds;
    FD_ZERO(&all_fds);
    FD_SET(STDIN_FILENO, &all_fds);
    FD_SET(sock_fd, &all_fds);

    printf("Please enter a username for the chat? ");
    char username[BUF_SIZE];
    scanf("%s", username);
    write(sock_fd, username, BUF_SIZE);
    // Read input from the user, send it to the server, and then accept the
    // echo that returns. Exit when stdin is closed.
    char buf[BUF_SIZE + 1];
    while (1) {
        fd_set set_copy = all_fds;
        int nready = select(sock_fd+1, &set_copy, NULL, NULL, NULL);
        if (nready == -1){
            perror("select");
            exit(1);
        }
        char buffer[BUF_SIZE];
        int end;
        if (FD_ISSET(sock_fd, &set_copy)){
            end = read(sock_fd, buffer, BUF_SIZE);
        } else if (FD_ISSET(STDIN_FILENO, &set_copy)){
            end = read(STDIN_FILENO, buffer, BUF_SIZE);
            buffer[end] = '\0';
            write(sock_fd, buffer, BUF_SIZE);
        } else {
            end = 0;
        }
        buffer[end] = '\0';

        printf("%s\n", buffer);

        int num_read = read(STDIN_FILENO, buf, BUF_SIZE);
        if (num_read == 0) {
            break;
        }
        buf[num_read] = '\0';         

        int num_written = write(sock_fd, buf, num_read);
        if (num_written != num_read) {
            perror("client: write");
            close(sock_fd);
            exit(1);
        }

        num_read = read(sock_fd, buf, BUF_SIZE);
        buf[num_read] = '\0';
        printf("Received from server: %s", buf);
    }

    close(sock_fd);
    return 0;
}
