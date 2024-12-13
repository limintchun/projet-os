#include <sys/socket.h> // socket(), setsockopt()
#include <netinet/in.h> // struct sockaddr_in, INADDR_ANY
#include <string.h>     // strlen()
#include <unistd.h>     // read(), write(), close()
#include <arpa/inet.h>  // inet_pton()
#include <stdio.h>      // printf(), perror()
#include <stdlib.h>     // getenv(), atoi()

int main() {
   
    // Initialisation du socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket()");
        return 1;
    }
   
    // Initialisation des paramètres de l'adresse
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr)); // Assurez-vous que tous les champs sont bien initialisés
    serv_addr.sin_family = AF_INET;

    // Récupération des variables d'environnement
    char *local_ip = getenv("IP_SERVEUR");
    char *local_port = getenv("PORT_SERVEUR");

    // Configuration de l'adresse IP
    if (local_ip == NULL) {
        fprintf(stderr, "IP_SERVEUR non initialisée. Utilisation de l'IP par défaut : 127.0.0.1\n");
        if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
            perror("inet_pton() pour l'adresse par défaut");
            close(sock);
            return 1;
        }
    } else {
        if (inet_pton(AF_INET, local_ip, &serv_addr.sin_addr) <= 0) {
            fprintf(stderr, "Adresse IP non valide. Utilisation de l'IP par défaut : 127.0.0.1\n");
            if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
                perror("inet_pton() pour l'adresse par défaut");
                close(sock);
                return 1;
            }
        }
    }
   
    // Configuration du port
    if (local_port == NULL) {
        fprintf(stderr, "PORT_SERVEUR non initialisé. Utilisation du port par défaut : 1234\n");
        serv_addr.sin_port = htons(1234);
    } else {
        int port = atoi(local_port);
        if (port >= 1 && port <= 65535) {
            serv_addr.sin_port = htons(port); // Conversion en format réseau
        } else {
            fprintf(stderr, "Port non valide. Utilisation du port par défaut : 1234\n");
            serv_addr.sin_port = htons(1234);
        }
    }

    // Connexion au serveur
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect()");
        close(sock);
        return 1;
    }
    printf("Connexion réussie au serveur %s:%d\n",
           local_ip ? local_ip : "127.0.0.1", ntohs(serv_addr.sin_port));

    // Envoi des données
    char message[1024];
    scanf("%s", message);
    if (write(sock, message, strlen(message)) < 0) {
        perror("write()");
        close(sock);
        return 1;
    }
  // Fermeture du socket
    close(sock);
    return 0;
}

