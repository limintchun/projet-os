#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>

#define PORT 1234
#define MAX_CLIENTS 100

// Structure pour chaque client
typedef struct {
    int sock;
    char pseudo[50];
} client_t;

client_t clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;  // Pour la protection des données clients

void *handle_client(void *arg) {
    int sock = *((int *)arg);
    char buffer[1024];
    int bytes_received;

    while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_received] = '\0';

        // Le message doit être sous la forme "[Nom destinataire] [Message]"
        char dest[50], message[1024];
        if (sscanf(buffer, "%s %[^\n]", dest, message) == 2) {
            // Trouver le client destinataire
            int found = 0;
            pthread_mutex_lock(&clients_mutex);  // Protection de l'accès aux données des clients
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].sock != 0 && strcmp(clients[i].pseudo, dest) == 0) {
                    // Envoie du message au destinataire
                    send(clients[i].sock, message, strlen(message), 0);
                    printf("Message envoyé à %s: %s\n", dest, message);
                    found = 1;
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);  // Libération du verrou
            if (!found) {
                // Si le destinataire n'est pas trouvé, envoyer un message d'erreur
                send(sock, "Destinataire introuvable.\n", 26, 0);
            }
        }
    }

    // Fermer la connexion du client et nettoyer
    close(sock);

    // Retirer le client de la liste des clients
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sock == sock) {
            clients[i].sock = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    return NULL;
}

void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("Signal SIGINT reçu, arrêt du serveur...\n");
        // Fermer tous les sockets clients et le socket serveur
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].sock != 0) {
                close(clients[i].sock);
            }
        }
        pthread_mutex_unlock(&clients_mutex);
        exit(0);
    }
}

int main() {
    signal(SIGINT, signal_handler);

    int server_fd, new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pthread_t thread_id;

    // Initialiser les clients à 0
    memset(clients, 0, sizeof(clients));

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    while (1) {
        new_sock = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (new_sock < 0) {
            perror("Accept failed");
            continue;
        }

        // Demander le pseudo du client
        char pseudo[50];
        recv(new_sock, pseudo, sizeof(pseudo), 0);
        printf("Client %s connecté.\n", pseudo);

        // Ajouter le client à la liste des clients
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].sock == 0) {
                clients[i].sock = new_sock;
                strncpy(clients[i].pseudo, pseudo, sizeof(pseudo));
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        // Créer un thread pour gérer ce client
        pthread_create(&thread_id, NULL, handle_client, &new_sock);
        pthread_detach(thread_id);
    }

    // Fermeture du socket serveur
    close(server_fd);
    return 0;
}
