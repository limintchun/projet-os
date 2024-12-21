#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "server.h"
#include "signals.h"

int main(void) {
   initializeSignalsHandler();

   int server_fd = create_socket();
   if (server_fd < 0) {
      perror("create_socket()");
      return 1;
   }

   struct sockaddr_in address;
   client_t client[MAX_CLIENT];
   initializeClients();
   initialize_address_data(&address);
   int client_fd = link_to_client(server_fd, &address);
   if (client_fd < 0) {
      perror("link_to_client()");
      return 1;
   }
   addClients(client_fd, address);

   pthread_t thread;
   int reader;

// write : je envoie au client ce que j'ai
   // read : je lis ce que le client m'envoie
   // comment différencier client A et B ?
   reader = pthread_create(&thread, NULL, read_thread, &client_fd);
   if (reader != 0) {
      perror("pthread_create()");
      return 1;
   }

   write_thread(&server_fd);
   pthread_join(thread, NULL);
   close(server_fd);
   close(client_fd);
   return 0;
}
