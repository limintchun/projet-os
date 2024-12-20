#include <sys/socket.h>  // socket(), setsockopt()
#include <netinet/in.h>   // struct sockaddr_in, INADDR_ANY
#include <string.h>       // strlen(), memset(), strpbrk()
#include <unistd.h>       // read(), write(), close()
#include <arpa/inet.h>    // inet_pton()
#include <stdio.h>        // printf(), perror()
#include <stdlib.h>       // getenv(), atoi()
#include <pthread.h>      // pthread_create(), pthread_join(), pthread_mutex_t
#include <signal.h>       // signal(), SIGINT, SIGPIPE
#include <stdbool.h>      // true false

#define MAX_MESSAGE_LENGTH 1024  // Longueur maximale des messages
#define MAX_PSEUDO_LENGTH 30     // Longueur maximale du pseudo utilisateur
#define BANNED_PUNCTUATION "./-[]" // Ponctuation bannie
#define TERMINAISON_0 1 // je rajoute + 1 a chaque fin de message pour que le compte de caractère soit correct et que le message fasse la longueur attendue

// Structure pour passer des informations au thread de réception des messages
typedef struct {
    int sock_fd;  // Socket pour la communication avec le serveur
} thread_args;

// Variables globales pour la gestion des options
bool is_bot = false;  // Si --bot est activé
bool is_manual = false;  // Si --manuel est activé

// Fonction pour gérer le signal SIGINT
void handle_sigint(int sig) {
    if (sig == SIGINT) {
        printf("\nDeconnecting...\n");
        exit(0);  // Quitter proprement en cas de signal d'interruption
    }
}

// Fonction pour gérer le signal SIGPIPE
void handle_sigpipe(int sig) {
    printf("\nConnection lost\n");
    exit(1);  // Quitter en cas de perte de connexion
}

// Fonction pour recevoir et afficher les messages du serveur
void *receive_messages(void *arg) {
    thread_args *args = (thread_args *)arg;
    int sock_fd = args->sock_fd;
    char buffer[MAX_MESSAGE_LENGTH + TERMINAISON_0];

    while (true) {
        memset(buffer, 0, MAX_MESSAGE_LENGTH + TERMINAISON_0);
        int bytes_read = read(sock_fd, buffer, MAX_MESSAGE_LENGTH);
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                printf("Server disconnected.\n");
            } else {
                perror("Error reading from server");
            }
            close(sock_fd);
            exit(1);
        }

        // -- manuel
        if (is_manual) {
            write(STDOUT_FILENO, "\a", 1);  // /a == beep sound
            // messages affichés dans la main
        } else {
            printf("%s", buffer);
        }
    }
    return NULL;
}

// Fonction pour envoyer un message au serveur
void send_message(int sock_fd, const char *message) {
    if (send(sock_fd, message, strlen(message), 0) == -1) {
        perror("Send message failed");
        close(sock_fd);

        exit(1);
    }
}

// Fonction pour valider le pseudo de l'utilisateur
int check_pseudo(const char *pseudo) {
    if (strlen(pseudo) > MAX_PSEUDO_LENGTH) {
        fprintf(stderr, "Error: pseudo too long (max %d characters).\n", MAX_PSEUDO_LENGTH);
        return 0;
    }
    
    //strpbrk -->
    if (strpbrk(pseudo, BANNED_PUNCTUATION) != NULL) {
        fprintf(stderr, "Error ; punctuation characters are forbidden for usernames.\n");
        return 0;
    }
    return 1;
}


int parse_argc(int argc){
    if (argc < 2) {
        fprintf(stderr, "Error: chat pseudo_utilisateur [--bot] [--manuel]\n");
        return -1;
    }
}


// Fonction principale du chat, gère la connexion et les interactions avec le serveur
void chat(int argc, char *argv[]) {
	if (parse_argc(argc) < 0){
		exit(1);
	}

    // Traitement des arguments
    const char *pseudo = argv[1];
    if (!check_pseudo(pseudo)) {
        exit(2);
    }

    // Traitement des options
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--bot") == 0) {
            is_bot = true;
        } else if (strcmp(argv[i], "--manuel") == 0) {
            is_manual = true;
        }
    }

    // Récupérer l'adresse IP et le port du serveur
    char *server_ip = getenv("IP_SERVEUR");
    if (server_ip == NULL) server_ip = "127.0.0.1";  // Adresse par défaut

    int server_port = 1234; // valeur par défaut
    char *env_port = getenv("PORT_SERVEUR"); //récup des variables d'environnement
    if (env_port != NULL) {
        server_port = atoi(env_port);
    }

    struct sockaddr_in server_addr; // struct qui contient le file_descriptor du socket
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(1);
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(1);
    }

    // Envoyer le pseudo au serveur pour peut etre plus de facilité
    //send_message(sock_fd, pseudo);

    // Créer les arguments pour le thread de réception
    thread_args args = {sock_fd};

    pthread_t read_thread;
    if (pthread_create(&read_thread, NULL, receive_messages, (void *)&args) != 0) {
        perror("Failed to create read thread");
        exit(1);
    }

    // Saisie de messages et envoi
    char message[sizeof(pseudo) + MAX_MESSAGE_LENGTH + TERMINAISON_0];
    char contenu_message[MAX_MESSAGE_LENGTH];

    while (true) {
        memset(message, 0, MAX_MESSAGE_LENGTH + TERMINAISON_0);
        if (fgets(contenu_message, MAX_MESSAGE_LENGTH, stdin) == NULL) perror("fgets");
        
        snprintf(message, sizeof(message), "[%s] %s", pseudo, contenu_message);
	
        // Ne pas envoyer un message vide
        if (message[0] != '\n') {
            send_message(sock_fd, message);
        }
    }

    pthread_join(read_thread, NULL);
    close(sock_fd);
}

// Fonction principale
int main(int argc, char *argv[]) {
    // Gestion des signaux SIGINT et SIGPIPE
    signal(SIGINT, handle_sigint);
    signal(SIGPIPE, handle_sigpipe);

    // Lancer le chat
    chat(argc, argv);

    return 0;
}
