#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#include "server.h"

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
client_t clients[MAX_CLIENT];
volatile sig_atomic_t sigint_catched = 0;
volatile sig_atomic_t sigpipe_catched = 0;

void sigintServ(int sig) {
   if (sigint_catched) {
      printf("Signal SIGINT reçu, arrêt du serveur\n");
      
      // tester si sans ça kick tlm
      pthread_mutex_lock(&clients_mutex);
      for (int i = 0; i < MAX_CLIENT; i++) {
         if (clients[i].socket_fd != 0) {
            close(clients[i].socket_fd);
         }
         else if (clients[i].connect != 1) {
            clients[i].connect = 0;
         }
      }
      pthread_mutex_unlock(&clients_mutex);
      printf("SIGINT: Programme arrêté");
      exit(1);
   }
}

void sigpipeSock(int signal) {
   // dans le cas ou il y'a plus de client connecté, close tout les socket proprement + printf("terminaison du programme") + lever sigpipe
   for (int i; i < MAX_SIZE; i++) {
      if (clients[i].connect == 0 && clients[i].socket_fd != -1) {
         close(clients[i].socket_fd);
      }
   }
   pthread_exit(NULL);
   printf("SIGPIPE: Programme arrêté");
}


void signalsHandler(int signal) {
   switch(signal) {
      case SIGINT: sigint_catched = 1;
         break;
      case SIGPIPE: sigpipe_catched = 1;
         break;
      default: break;
   }
}
void inititializeSignalsHandler() {
   struct sigaction action;
   action.sa_handler = signalsHandler;
   action.sa_flags = 0;
   sigemptyset(&action.sa_mask);
  
   if (sigaction(SIGINT, &action, NULL) < 0) {
      perror("\nFailed to set handler for SIGINT");
   }
   else if (sigaction(SIGPIPE, &action, NULL) < 0) {
      perror("\nFailed to set handler for SIGPIPE");
   }
}
int main() {
   return 0;
}
