#include <stdio.h> // printf()
#include <sys/types.h>
#include <sys/socket.h> // socket(), setsockopt()
#include <netinet/in.h> // struct sockaddr_in, INADDR_ANY
#include <unistd.h> // read(), write(), close()
#include <arpa/inet.h> // inet_pton (peut-être à supprimer)
#include <string.h> // strlen
#include <stdlib.h> // getenv()


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
    serv_addr.sin_port = htons(8080);
    // char *local_ip = getenv("IP_SERVEUR");
    // char *local_port = getenv("PORT_SERVEUR");

    // initialisation de l'adresse ip en fonction de la variable locale
    // if (local_ip == NULL) {
    //     printf("Adresse IP non initialisée"); // pour que ça fonctionne, utiliser export VAR=VALEUR
    //     return 1;
    // }
    // else {
    //     struct in_addr ip_address;
    //     int ip = inet_aton(local_ip, &ip_address);
    //     if () {

    //     }
    //     else {
    //         serv_addr.sin_addr.s_addr = inet_aton("127.0.0.1", &ip_address);
    //     }
    // }

    // initialisation du port en fonction de la variable locale
    // if (local_port == NULL) {
    //     printf("Port non initialisée");
    //     return 1;
    // }
    // else {
    //     int port = atoi(local_port); // permet de convertir char vers int
    //     if (port > 1 && port < 65535) {
    //         serv_addr.sin_port = htons(port); // convertit en format réseau
    //     }
    //     else {
    //         serv_addr.sin_port = htons(1234);
    //     }

    // }


    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr); // peut-être à supprimer
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    char message[] = "Salut à tous";
    write(sock, message, strlen(message));
    close(sock);

    return 0;
}
