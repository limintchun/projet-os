#include <stdio.h>
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

void defaultMode(char username[], char message[]);
void botMode(char username[], char message[]);
void manuelMode(char username[], char message[], int sig);

void defaultMode(char username[], char message[]) {
   printf("[" ANSI_COLOR_YELLOW"\x1B[4m%s\x1B[0m] %s\n" ANSI_COLOR_RESET, username, message);
}

void manuelMode(char username[], char message[], int sig) {
   if (sig == SIGINT) {
      printf("user pressed CTRL-C");
   }

}


int main(int argc, char* argv[]) {

   if (argc > 1) {
      if (strlen(argv[1]) > TAILLE_MAX || strlen(argv[2]) > TAILLE_MAX) {
      fprintf(stderr, "un pseudonyme doit contenir maximum 30 caractères\n");
      return 2;
      }
   }
   else {
      fprintf(stderr, "chat pseudo_utilisateur pseudo_destinataire [--bot] [--manuel]\n");
      return 1;
   }
   
   pid_t process = fork();
   if (process > 0) { // process qui lit les msgs
      printf("Veuillez entrer votre message");
      
      char buffer[1000];
      fgets(buffer, sizeof(buffer), stdin);
   }
   else if (process == 0) { // process qui affiche ce qui est lu sur le pipe
      printf("je suis le fils %d\n", getpid());
   }
   else {
      perror("fork()");
      return 1;
   }
   
   return 0;
}
