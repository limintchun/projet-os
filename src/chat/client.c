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

<<<<<<< HEAD
// Fonction pour le thread d'écriture
void* write_thread(int sock) {
    char message[1024];
    while (1) {
        printf("Entrez un msg :\n");
        if (fgets(message, sizeof(message), stdin) != NULL) {
            perror("fgets()");
        }
        // scanf("%s", message);

        message[strcspn(message, "\n")] = '\0';
        // Verrouiller le mutex avant d'écrire
        //pthread_mutex_lock(&lock);
        if (write(sock, message, strlen(message)) < 0) {
            perror("write()");
        }
        // Déverrouiller le mutex après l'écriture
        //pthread_mutex_unlock(&lock);
    }
=======
// Déclaration de la structure client_data avant les fonctions
typedef struct {
    bool bot_mode;  // Si --bot est activé
    bool manual_mode;  // Si --manuel est activé
    int socket; // file descriptor socket
} client_data;

//déclaration des fonctions
void handle_sigint(int sig);
void handle_sigpipe(int sig);

void *receive_messages(void *arg);
void send_message(int sock_fd, const char *message);
int check_pseudo(const char *pseudo);
void parse_argc(int argc);
void mode_detection(int argc, char *argv[], client_data *data);
void init_data(client_data *data);
void set_port_ip(char **server_ip, int *server_port);
void chat(int argc, char *argv[]);
int main(int argc, char *argv[]);

// Fonction pour gérer le signal SIGINT si le client se déconnecte avec CTRL + C
void handle_sigint(int sig) {
    if (sig == SIGINT) {
        printf("\nDeconnecting...\n");
        exit(0);  // Quitter proprement en cas de signal d'interruption
    }
}

// Fonction pour gérer le signal SIGPIPE --> si la connexion est coupée avec le serveur
void handle_sigpipe(int sig) {
    printf("\nConnection lost\n");
    exit(1);  // Quitter en cas de perte de connexion
}

// Fonction pour recevoir et afficher les messages du serveur
void *receive_messages(void *arg) {
    client_data *data = (client_data *)arg;  // Cast de l'argument en pointeur vers client_data
    int sock_fd = data->socket; // récupération du socket_fd

    char buffer[MAX_MESSAGE_LENGTH + TERMINAISON_0]; // 1024 + 1 octet pour /0

    while (true) {
        memset(buffer, 0, MAX_MESSAGE_LENGTH + TERMINAISON_0); // initialisation à 0
        int bytes_read = read(sock_fd, buffer, MAX_MESSAGE_LENGTH);
        if (bytes_read <= 0) {//gestion d'erreurs
            if (bytes_read == 0) {
                printf("Server disconnected.\n");
            } else {
                perror("Error reading from server");
            }
            close(sock_fd);//fermeture du socket
            exit(1);
        }

        if (data->manual_mode) { // manuel
            write(STDOUT_FILENO, "\a", 1);  // /a == beep sound
        } else {
            printf("%s", buffer); // messages affichés normalement
        }
    }

>>>>>>> 147445e58eb7c8c317aa3e2434930b1f1b527992
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

    if (strpbrk(pseudo, BANNED_PUNCTUATION) != NULL) { // strpbrk compare des chaines et return 1 si un caractère est le même
        fprintf(stderr, "Error: punctuation characters are forbidden for usernames.\n");
        return 0;
    }
    return 1;
}

// Fonction pour traiter les arguments
void parse_argc(int argc) {
    if (argc < 2) {
        fprintf(stderr, "Error: chat pseudo_utilisateur [--bot] [--manuel]\n");
        exit(1);
    }
}

// Fonction pour détecter les modes
void mode_detection(int argc, char *argv[], client_data *data) {
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--bot") == 0) {
            data->bot_mode = true;
        } else if (strcmp(argv[i], "--manuel") == 0) {
            data->manual_mode = true;
        }
    }
}

// Initialisation des données
void init_data(client_data *data) {
    data->bot_mode = false;  // Initialiser à false par défaut
    data->manual_mode = false; // Initialiser à false par défaut
    data->socket = 0;
}

// Fonction pour récupérer l'IP et le port
void set_port_ip(char **server_ip, int *server_port) {
    *server_ip = getenv("IP_SERVEUR");
    if (*server_ip == NULL) *server_ip = "127.0.0.1";  // Adresse par défaut

    *server_port = 1234; // valeur par défaut
    char *env_port = getenv("PORT_SERVEUR");
    if (env_port != NULL) {
        *server_port = atoi(env_port);
    }
}

void chat(int argc, char *argv[]) {
    // Allocation dynamique pour la structure client_data parce que sans cela j'obtenais Segmentation fault (core dumped)
    // qui est lié à un soucis de pointeurs et de l'allocation en mémoire. Efffectivement lorque je lancais init_data l'erreur se produisait avec Segmentation fault (core dumped)
    //  du à la signature de ma fonction qui est un pointeur vers un pointeur
    client_data *data = (client_data *)malloc(sizeof(client_data));
    if (data == NULL) {
        perror("Failed to allocate memory for client_data");
        exit(1);
    }

    parse_argc(argc);

    // Traitement des arguments
    const char *pseudo = argv[1];
    if (!check_pseudo(pseudo)) {
        free(data); // Libérer la mémoire allouée avant de quitter
        exit(2);
    }

    // Initialisation de la struct contenant les données du client
    init_data(data);

    // Détecte si des modes sont activés
    mode_detection(argc, argv, data);

    char *server_ip;
    int server_port;

    set_port_ip(&server_ip, &server_port);

    struct sockaddr_in server_addr;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        free(data); // Libérer la mémoire allouée en cas d'échec
        exit(1);
    }
    
    // Activer SO_REUSEADDR pour permettre de réutiliser immédiatement le port après une fermeture
    int opt = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(sock_fd);
        free(data); // Libérer la mémoire allouée en cas d'échec
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        free(data); // Libérer la mémoire allouée en cas d'échec
        exit(1);
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        free(data); // Libérer la mémoire allouée en cas d'échec
        exit(1);
    }

    // initialiser la valeur de sock_fd dans la struct
    data->socket = sock_fd;

    pthread_t read_thread;
    if (pthread_create(&read_thread, NULL, receive_messages, (void *)data) != 0) {
        perror("Failed to create read thread");
        free(data); // Libérer la mémoire allouée en cas d'échec
        exit(1);
    }

    // Thread principal chargé de l'envoi des messages
    char message[MAX_PSEUDO_LENGTH + MAX_MESSAGE_LENGTH + TERMINAISON_0];
    char contenu_message[MAX_MESSAGE_LENGTH];

    while (true) {
        memset(message, 0, sizeof(message));
        if (fgets(contenu_message, MAX_MESSAGE_LENGTH, stdin) == NULL) {
            perror("fgets");
        }

        snprintf(message, sizeof(message), "[%s] %s", pseudo, contenu_message);

        if (message[0] != '\n') {
            send_message(sock_fd, message);
        }
    }

    pthread_join(read_thread, NULL);
    close(sock_fd);
    free(data); // Libérer la mémoire allouée à la fin du programme
}


<<<<<<< HEAD
    // Initialiser le mutex
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Échec de l'initialisation du mutex\n");
        return 1;
    }

    // Créer les threads
    // fonction lancée lancé par le thread d'origine aka main()
    write_thread(sock);

    // fonction lancée par le second thread
    pthread_t reader;
    if (pthread_create(&reader, NULL, read_thread, &sock) != 0) {
        printf("Échec de la création du thread de lecture\n");
        return 1;
    }

    // Attendre la fin des threads
    pthread_join(reader, NULL);

    // Détruire le mutex
    pthread_mutex_destroy(&lock);

    // Fermeture du socket
    close(sock);
=======
// Fonction principale
int main(int argc, char *argv[]) {
    signal(SIGINT, handle_sigint);
    signal(SIGPIPE, handle_sigpipe);
    chat(argc, argv);
>>>>>>> 147445e58eb7c8c317aa3e2434930b1f1b527992
    return 0;
}
