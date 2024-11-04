#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define TAILLE_MAX 30

bool contains(char name[], char value);
// à déterminer si utile void défaut(char username, char message);
// à déterminer si utile void manuel(int size, int taille); 

bool contains(char name[], char value) { // NOTE: le mot-clé "in" n'existe pas en c
   for (int i = 0; i < strlen(name); i++) {
      if (name[i] == value) {
         return true;
      }
   }
   return false;
}


int main(int argc, char* argv[]) {
   char forbidden[] = {'/', '-', '[', ']', '.', '..' };

   if (argc > 1) {
      if (strlen(argv[1]) > TAILLE_MAX || strlen(argv[2]) > TAILLE_MAX) {
      fprintf(stderr, "un pseudonyme doit contenir maximum 30 caractères\n");
      return 2;
      }

//      for (int i = 0; i < sizeof(forbidden); i++) {
//         if (contains(argv[0], forbidden[i])) {
//            fprintf(stderr, "vous ne pouvez pas utiliser le caractère suivant : %s", &forbidden[i]);
//            return 3;
//         }
//         else if (contains(argv[1], forbidden[i])) {
//            fprintf(stderr, "vous ne pouvez pas utiliser le caractère suivant : %s", &forbidden[i]);
//         }
//      }
   }
   else {
      fprintf(stderr, "chat pseudo_utilisateur pseudo_destinataire [--bot] [--manuel]\n");
      return 1;
   }


   pid_t process = fork();
   if (process > 0) { // process qui lit les msgs
      printf("je suis le père %d\n", getpid());
      
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
