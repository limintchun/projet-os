#include <stdio.h>       // printf(), perror()
#include <stdlib.h>      // getenv(), atoi()
#include <unistd.h>      // read(), write(), close()
#include <sys/socket.h>  // socket(), setsockopt(), bind(), listen(), accept()
#include <netinet/in.h>  // struct sockaddr_in, INADDR_ANY
#include <string.h>      // memset()
#include <pthread.h> // pthread_t, pthread_create(), pthread_join()

#include "server.h"

#define DEFAULT_PORT 1234
#define MAX_SIZE 1024
#define MAX_CLIENT 1000
#define MAX_CHAR 30
#define DECONNECTED 0
#define CONNECTED 1 

client_t clients[MAX_CLIENT];
int count = 0;

// on initialise une liste de client en partant du principe qu'ils ne sont pas encore connecté
void initializeClients() {
    for (int i = 0; i < MAX_CLIENT; i++) {
        clients[i].connect = DECONNECTED;
        clients[i].socket_fd = -1;
    }
}

void addClients(int client_fd, struct sockaddr_in client_addr) {
    socklen_t addrlen = sizeof(client_addr);
    count ++;
    if (clients[count].connect == 0) {
        clients[count].socket_fd = client_fd;
        clients[count].addr = client_addr;
        clients[count].connect = CONNECTED;
    }
}
// faire une fonction cherchant le client + socket correspondant
void searchClient(char message[MAX_SIZE], struct sockaddr_in client_addr) {
    for (int i = 0; i < MAX_CLIENT; i++) {
        // clients[count].port = 
    }
}

int create_socket() {
    // Initialisation du socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket()");
        return -1;
    }

    // Ajout d'option au socket, comme la capacité de réutiliser une adresse ou un port
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < -1) {
        perror("setsockopt()");
        close(server_fd);
        return -1;
    }

    return server_fd;
}

void initialize_address_data(struct sockaddr_in *address) {
    // Initialisation de la structure contenant les informations du socket
    memset(address, 0, sizeof(*address)); // Remplissage initial à 0 pour éviter les erreurs
    address->sin_family = AF_INET;         // Toujours AF_INET pour les adresses Internet
    address->sin_addr.s_addr = INADDR_ANY; // Écouter sur toutes les interfaces réseau disponibles

    // Récupération de la variable d'environnement pour le port
    char *local_port = getenv("PORT_SERVEUR"); // En bash, utilisez export VARIABLE=VALEUR
    if (local_port == NULL) {
        printf("PORT_SERVEUR non initialisé. PORT_SERVEUR défini à 1234\n");
        address->sin_port = htons(DEFAULT_PORT);
        local_port = "1234";
    }

    // Conversion de la variable d'environnement en entier et validation
    int port = atoi(local_port);
    if (port >= 1 && port <= 65535) {
        address->sin_port = htons(port); // Convertir en format réseau
    } else {
        printf("PORT_SERVEUR invalide. Utilisation du port par défaut : 1234.\n");
        address->sin_port = htons(DEFAULT_PORT); // Port par défaut
    }
}

int link_to_client(int server_fd, struct sockaddr_in *address) {
    // Liaison du socket à une adresse et un port
    if (bind(server_fd, (struct sockaddr *)address, sizeof(*address)) < 0) {
        perror("bind()");
        close(server_fd);
        return -1;
    }

    // Écoute sur le socket pour les demandes de connexion
    if (listen(server_fd, 5) < 0) { // Longueur de la file d'attente de connexion fixée à 5
        perror("listen()");
        close(server_fd);
        return -1;
    }
    printf("Serveur en écoute sur le port %d...\n", ntohs(address->sin_port));

    // Attente d'une connexion client
    socklen_t addrlen = sizeof(*address);
    int new_socket = accept(server_fd, (struct sockaddr *)address, &addrlen);
    if (new_socket < 0) {
        perror("accept()");
        close(server_fd);
        return -1;
    }
    printf("Connexion acceptée.\n");

    return new_socket;
}


// je reprend la même fct que dans client sauf que le serveur ne devra pas afficher les message mais les rediriger dans un autre socket
void* write_thread(void* server_fd) {
    int sock = *(int*)server_fd;
    char message[MAX_SIZE];
    //while (1) {
        // scanf("%s", message);
        // Verrouiller le mutex avant d'écrire
        //pthread_mutex_lock(&lock);
        if (write(sock, message, strlen(message)) < 0) {
            perror("write()");
        }
        // Déverrouiller le mutex après l'écriture
        //pthread_mutex_unlock(&lock);
    //}
    return NULL;
}

// Fonction pour le thread de lecture
void* read_thread(void* client_fd) {
    int sock = *(int*)client_fd;
    char buffer[MAX_SIZE];
    //while (1) {
        // Verrouiller le mutex avant de lire
        //pthread_mutex_lock(&lock);
        int bytes_read = read(sock, buffer, sizeof(buffer) - 1);
        if (bytes_read < 0) {
            perror("read()");
        } else {
            buffer[bytes_read] = '\0'; // notifier la fin du message pour éviter la lecture de caractère non-désirés
            printf("Message du client : %s\n", buffer);
        }
        // Déverrouiller le mutex après la lecture
      //  pthread_mutex_unlock(&lock);
    //}
    return NULL;
}
