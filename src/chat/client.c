#include <sys/socket.h> // socket(), setsockopt()
#include <netinet/in.h> // struct sockaddr_in, inaddr_any
#include <string.h> // strlen()
#include <unistd.h> // read(), write(), close()
#include <arpa/inet.h> // inet_pton()
#include <stdio.h> // printf()
#include <stdlib.h> // getenv()
#include <sys/types.h>

int main() {

    // initialisation du socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket()");
        return 1;
    }

    // initialisation des paramètres de l'adresse
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;

    // récupération des variables locales
    char *local_ip = getenv("IP_SERVEUR");
    char *local_port = getenv("PORT_SERVEUR");

    // initialisation de l'adresse ip en fonction de la variable locale
    // if (local_ip == NULL) {
    //     printf("IP_SERVEUR non initialisée"); // pour que ça fonctionne, utiliser export VAR=VAL
    //     return 1;
    // }
    // else {
    //     if (inet_pton(AF_INET, local_ip, &serv_addr.sin_addr) < 0) {
    //         perror("inet_pton()");
    //         return 1;
    //     }
    //     else if (inet_pton(AF_INET, local_ip, &(serv_addr.sin_addr)) == 1) { // adresse ip valide
    //         serv_addr.sin_addr.s_addr = inet_pton(AF_INET, local_ip, &serv_addr.sin_addr);
    //     }
    //     else { // adresse ip non valide
    //         serv_addr.sin_addr.s_addr = inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr); 
    //     }
    // }

    // initialisation du port en fonction de la variable locale
     if (local_port == NULL) {
         printf("PORT_SERVEUR non initialisée");
         return 1;
     }
     else {
         int port = atoi(local_port); // permet de convertir char vers int
         if (port >= 1 && port <= 65535) {
             serv_addr.sin_port = htons(port); // convertit en format réseau
         }
         else {
             serv_addr.sin_port = htons(1234);
         }

     }

    // connexion au socket
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect()");
        return 1;
    }

    // écriture des données
    char message[] = "SALUT A TOUS";
    if (write(sock, message, strlen(message)) < 0) {
        perror("write()");
        return 1;
    }

    // fermeture du socket
    close(sock);
    return 0;
}
