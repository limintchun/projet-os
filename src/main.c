#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>


#define MAX_LENGHT_USERNAME 30


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


void handler(int received_signal) {
    if (received_signal == SIGINT) {
      printf("  \n"); //evite que le message d'erreur soit collé à l'éventuel message écrit
      printf("The user pressed CTRL+C.\n");
      exit(0);
    }
}

int main(int argc, char* argv[]) {
  if (argc > 1){

    //check longueur pseudos
    if (strlen(argv[0]) > TAILLE_MAX || strlen(argv[1]) > TAILLE_MAX) {
      fprintf(stderr, "un pseudonyme doit contenir maximum 30 caractères\n");
      return 2;

    // lien avec code Ayaman checke si paramètres doivent être considéres en pseudos
    }else if (strcmp(argv[i], "--bot") == 0) {
       continue;

    }else if (strcmp(argv[i], "--manuel") == 0) {
       continue;

    }else:
      for (int num_pseudo = 1; num_pseudo < 3; num_pseudo++) {
        // Vérification parmi les deux pseudos si des caractères spéciaux sont présents
        for (int charactere = 0; charactere < strlen(argv[num_pseudo]); charactere++) {
          char c = argv[num_pseudo][charactere];  // Récupère le caractère actuel

          // Vérification si le caractère est un des symboles recherchés
          if (c == '/' || c == '-' || c == '[' || c == ']' || c == '.') {
            fprintf(stderr, "Failure to create username, the pseudo %d  cannot contain the character %c\n",num_pseudo, c);
            return 3;
          }
        }
      }
  }
  if(argc == 4){
    if(argv[3] == "--bot"){


    }

  }
  return 0;
}
