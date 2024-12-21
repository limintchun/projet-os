#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "../serveur/server.c"

#define MAX_SIZE 1024

void *inputUser(char buffer[MAX_SIZE]) {
    if (fgets(buffer, MAX_SIZE, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len -1] = '\0';
        }
    }
    return buffer; 
}

void *readSock(int sock_fd) {
    if (read(sock_fd) == -1) {
        perror("read()");
    }
}

int main() {
    char buffer[MAX_SIZE];
    pthread_t thread;

    int readSock = pthread_create(&thread, NULL, &buffer);

}
