#include <sys/socket.h> // socket(), setsockopt()
#include <netinet/in.h> // struct sockaddr_in, INADDR_ANY
#include <string.h>     // strlen()
#include <unistd.h>     // read(), write(), close()
#include <arpa/inet.h>  // inet_pton()
#include <stdio.h>      // printf(), perror()
#include <stdlib.h>     // getenv(), atoi()
#include <pthread.h>    // pthread_create(), pthread_join(), pthread_mutex_t

// Déclaration du mutex
pthread_mutex_t lock;

// Fonction pour le thread d'écriture
void* write_thread(void* arg) {
    int sock = *(int*)arg;
    char message[1024];
    while (1) {
        printf("Entrez un message : ");
        scanf("%s", message);

        // Verrouiller le mutex avant d'écrire
        pthread_mutex_lock(&lock);
        if (write(sock, message, strlen(message)) < 0) {
            perror("write()");
        }
        // Déverrouiller le mutex après l'écriture
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

// Fonction pour le thread de lecture
void* read_thread(void* arg) {
    int sock = *(int*)arg;
    char buffer[1024];
    while (1) {
        // Verrouiller le mutex avant de lire
        pthread_mutex_lock(&lock);
        int bytes_read = read(sock, buffer, sizeof(buffer) - 1);
        if (bytes_read < 0) {
            perror("read()");
        } else {
            buffer[bytes_read] = '\0';
            printf("Message du serveur : %s\n", buffer);
        }
        // Déverrouiller le mutex après la lecture
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main() {
    // Initialisation du socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket()");
        return 1;
    }

    // Initialisation des paramètres de l'adresse
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;

    // Récupération des variables d'environnement
    char *local_ip = getenv("IP_SERVEUR");
    char *local_port = getenv("PORT_SERVEUR");

    // Configuration de l'adresse IP
    if (local_ip == NULL) {
        printf("IP_SERVEUR non initialisée. L'adresse ip est initialisé par défaut à 127.0.0.1\n");
        inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    } else {
        if (inet_pton(AF_INET, local_ip, &serv_addr.sin_addr) == 1) {
            inet_pton(AF_INET, local_ip, &serv_addr.sin_addr);
        } else if (inet_pton(AF_INET, local_ip, &serv_addr.sin_addr) == 0) {
            inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
        }
    }

    // Configuration du port
    if (local_port == NULL) {
        printf("PORT_SERVEUR non initialisé. PORT_SERVEUR défini à 1234\n");
        serv_addr.sin_port = htons(1234);
    } else {
        int port = atoi(local_port);
        if (port >= 1 && port <= 65535) {
            serv_addr.sin_port = htons(port); // Convertir en format réseau
        } else {
            printf("PORT_SERVEUR invalide. Utilisation du port par défaut : 1234.\n");
            serv_addr.sin_port = htons(1234); // Port par défaut
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

    // Initialiser le mutex
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Échec de l'initialisation du mutex\n");
        return 1;
    }

    // Créer les threads
    pthread_t writer, reader;
    if (pthread_create(&writer, NULL, write_thread, &sock) != 0) {
        printf("Échec de la création du thread d'écriture\n");
        return 1;
    }
    if (pthread_create(&reader, NULL, read_thread, &sock) != 0) {
        printf("Échec de la création du thread de lecture\n");
        return 1;
    }

    // Attendre la fin des threads
    pthread_join(writer, NULL);
    pthread_join(reader, NULL);

    // Détruire le mutex
    pthread_mutex_destroy(&lock);

    // Fermeture du socket
    close(sock);
    return 0;
}
