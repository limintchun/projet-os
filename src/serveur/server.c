#include <stdio.h>       // printf(), perror()
#include <stdlib.h>      // getenv(), atoi()
#include <unistd.h>      // read(), write(), close()
#include <sys/socket.h>  // socket(), setsockopt(), bind(), listen(), accept()
#include <netinet/in.h>  // struct sockaddr_in, INADDR_ANY
#include <string.h>      // memset()

#define MAX_SIZE 1024

int main() {
    // Initialisation du socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket()");
        return 1;
    }

    // Ajout d'option au socket, comme la capacité de réutiliser une adresse ou un port
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < -1) {
        perror("setsockopt()");
        close(server_fd);
        return 1;
    }
    // Initialisation de la structure contenant les informations du socket
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address)); // Remplissage initial à 0 pour éviter les erreurs
    address.sin_family = AF_INET;         // Toujours AF_INET pour les adresses Internet
    address.sin_addr.s_addr = INADDR_ANY; // Écouter sur toutes les interfaces réseau disponibles

    // Récupération de la variable d'environnement pour le port
    char *local_port = getenv("PORT_SERVEUR"); // En bash, utilisez export PORT_SERVEUR=1234
    if (local_port == NULL) {
        printf("PORT_SERVEUR non initialisé. PORT_SERVEUR défini à 1234\n");
        address.sin_port = htons(1234);
        local_port = "1234";
    }

    // Conversion de la variable d'environnement en entier et validation
    int port = atoi(local_port);
    if (port >= 1 && port <= 65535) {
        address.sin_port = htons(port); // Convertir en format réseau
    } else {
        printf("PORT_SERVEUR invalide. Utilisation du port par défaut : 1234.\n");
        address.sin_port = htons(1234); // Port par défaut
    }

    // Liaison du socket à une adresse et un port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind()");
        close(server_fd);
        return 1;
    }

    // Écoute sur le socket pour les demandes de connexion
    if (listen(server_fd, 5) < 0) { // Longueur de la file d'attente de connexion fixée à 5
        perror("listen()");
        close(server_fd);
        return 1;
    }
    printf("Serveur en écoute sur le port %d...\n", ntohs(address.sin_port));

    // Attente d'une connexion client
    socklen_t addrlen = sizeof(address);
    int new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    if (new_socket < 0) {
        perror("accept()");
        close(server_fd);
        return 1;
    }
    printf("Connexion acceptée.\n");

    // Lecture des données envoyées par le client
    char buffer[MAX_SIZE] = {0}; // Initialisation à 0 pour éviter des caractères parasites
    ssize_t bytes_read = read(new_socket, buffer, MAX_SIZE - 1); // MAX_SIZE - 1 pour garder la place pour le '\0'
    if (bytes_read < 0) {
        perror("read()");
        close(new_socket);
        close(server_fd);
        return 1;
    }
    printf("Message reçu : %s\n", buffer);

    // Fermeture des sockets
    close(new_socket);
    close(server_fd);

    return 0;
}

