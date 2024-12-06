#include <stdio.h> // printf()
#include <sys/types.h>
#include <sys/socket.h> // socket(), setsockopt()
#include <netinet/in.h> // struct sockaddr_in, INADDR_ANY
#include <unistd.h> // read(), write()
#include <stdlib.h> // getenv()

#define MAX_SIZE 1024

int main() {

   // initialisation du socket
   int server_fd= socket(AF_INET, SOCK_STREAM, 0);
   if (server_fd < 0) {
      perror("socket()");
      return 1;
   }

   // ajout d'option au sockets, comme la capacité de réutilisé une addresse ou un port
   int opt = 1;
   if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < -1) {
      perror("setsockopt()");
      return 1;
   }

   // initialisation de la structure contenant les informations du sockets
   struct sockaddr_in address;
   address.sin_family = AF_INET; // toujours AF_INT car identifiant des familles d'adresses internet
   address.sin_addr.s_addr = INADDR_ANY;

   // récupération de la variable locale
   char *local_port = getenv("PORT_SERVEUR"); // en bash, utiliser export NOM_VARIABLE=VALEUR
   // initialisation du port en fonction de la variable locale
   if (local_port == NULL) {
      printf("Port non initialisé");
      return 1;
   }
   else {
   int port = atoi(local_port); // permet de convertir char vers int
      if (port >= 1 && port <= 65535) {
         address.sin_port = htons(port); // convertit en format réseau
      }
      else {
         address.sin_port = htons(1234);
      }
   }

   // liaison du socket à un port
   if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
      perror("bind()");
      return 1;
   }

   // écoute sur le socket server_fd les demandes de connexion
   if (listen(server_fd, 5) < 0) { // 5 car la longueur de la file d'attente pour les nouvelles connexions est 5
      perror("listen()");
      return 1;
   }

   // connexion au socket et création d'un nouveau socket pour communiquer avec le client
   size_t addrlen = sizeof(address);
   int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *) &addrlen);
   if (new_socket < 0) {
      perror("accept()");
      return 1;
   }
   
   // lecture de data
   char buffer[MAX_SIZE];
   if (read(new_socket, buffer, MAX_SIZE) < 0) {
      perror("read()");
      return 1;
   }
   printf("Message reçu : %s\n", buffer);

   // fermeture des sockets
   close(new_socket);
   close(server_fd);

   return 0;
}
