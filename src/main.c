#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#define TAILLE_MAX 30
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
// déterminer si on veut utiliesr une  couleur, si oui, faire la différence entre sender & receiver ?
//
// NOTE: tjrs vérifier les valeurs de retour de read & write pour les pipes nommés
// pour read : 
//    if ret_value > 0 : nb d'octets lus 
//    elif ret_value < 0 : erreur / SIGPIPE 
//    elif ret_value = 0 : pas de flux
// pour write : 
//    if ret_value > 0 : nb d'octets écrits 
//    elif ret_value < 0 : erreur / SIGPIPE (càd pas de lecture) -> errno = EPIPE
// créer des fonctions pour gérer ça
// utiliser sig_atomic_t ? 

void affichage(char username[], char message[], bool modificationActivate);
void botMode(char username[], char message[]);
void manuelMode(char username[], char message[], int sig);
void parent(int write_fd);
void fils(int read_fd, char sender[]);

void affichage(char username[], char message[], bool modificationActivate) {
   if (modificationActivate == true) {
      printf("[%s] : %s", username, message);
   }
   else {
      printf("[" ANSI_COLOR_YELLOW"\x1B[4m%s\x1B[0m] %s\n" ANSI_COLOR_RESET, username, message);
   }
}

void parent(int write_fd) {

   printf("je suis le père");
   printf("Veuillez entrer votre message : ");

   char buffer[256];
   while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
      write(write_fd, buffer, strlen(buffer) + 1);
   };
}

void fils(int read_fd, char sender[]) {

   printf("je suis le fils \n");

   char buffer[256];
   while (read(read_fd, buffer, sizeof(buffer)) > 0) {
      affichage(sender, buffer, false);
   }

}

// j'essaye de comprendre comment les signaux fonctionne
//void handler(int sig) {
//   if (sig == SIGINT) {
//      printf("l'utilisateur a appuyé sur ctrl+c\n");
//      exit(0);
//   }
//   else if (sig == SIGPIPE) {
//      printf("signal pipe");
//      exit(0);
//   }
//}


int main(int argc, char* argv[]) {

   // Récupération des pseudonymes ; même si les pseudos sont "--bot" ou "--manuel", le programme considérera de la même manière les variables
   char *sender = argv[1];
   char *receiver = argv[2];

   // Gestion de paramètres attribués au programme
   if (argc > 2) {
      if (strlen(argv[1]) > TAILLE_MAX || strlen(argv[2]) > TAILLE_MAX) {
         fprintf(stderr, "un pseudonyme doit contenir maximum 30 caractères\n");
         return 2;
      }
   }
   else {
      fprintf(stderr, "chat pseudo_utilisateur pseudo_destinataire [--bot] [--manuel]\n");
      return 1;
   }

   // Création de la pipe nommée + gestion d'erreur de sa création +    
   char sender_fifo[256]; 
   char receiver_fifo[256];
   sprintf(sender_fifo, "/tmp/%s-%s.chat", sender, receiver);
   sprintf(receiver_fifo, "/tmp/%s-%s.chat", receiver, sender);
   printf("%s, %s", sender_fifo, receiver_fifo);

   // supprime les fichiers correspondant aux pipes nommées si elles exsitent déjà
   unlink(sender_fifo);
   unlink(receiver_fifo);

   if (mkfifo(sender_fifo, 0666) == -1) {
      perror("mkfifo()");
      return 1;
   }
   else if (mkfifo(receiver_fifo, 0666) == -1) {
      perror("mkfifo()");
      return 1;
   }

   // création de processus
   pid_t process = fork();
   if (process > 0) { // process qui lit les msgs
      int fd_write = open(sender_fifo, O_WRONLY); // ouverture du pipe nommé après le fork D'APRES chatgpt
      if (fd_write == -1) {
         perror("échec d'ouverture du pipe : ");
         unlink(sender_fifo);
         unlink(receiver_fifo);
         return 1;
      }

      parent(fd_write);
      close(fd_write);

      wait(NULL);// attente du processus fils D'APRES chatgpt unlink(sender_fifo);
      unlink(sender_fifo);
      unlink(receiver_fifo);
   }

   else if (process == 0) { // process qui affiche ce qui est lu sur le pipe
      int fd_read = open(receiver_fifo, O_RDONLY); // same
      if (fd_read == -1) {
         perror("échec d'ouverture du pipe : ");
         return 1;
      }

      fils(fd_read, sender);


      unlink(sender_fifo);
      unlink(receiver_fifo);
   }
   else {
      perror("fork()");
      return 1;
   }

   return 0;
}
