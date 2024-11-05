#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>


// CamelCase for struct
// camelCase for function
// under_score for variable


#define MAX_LENGHT_USERNAME 30
#define BOT_MODE "--bot"
#define MANUAL_MODE "--manuel"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


int checkVadlidArgv(int argc, char* argv[], bool* bot_mode_activated, bool* manual_mode_activated);
void myHandler(int received_signal);


int checkVadlidArgv(int argc, char* argv[], bool* bot_mode_activated, bool* manual_mode_activated) {
   if (1 == argc)  {
      fprintf(stderr, "chat pseudo_utilisateur pseudo_destinataire [--bot] [--manuel]\n");
      return 1;
   }
   else {
      for (int i = 1; i < 3; i++) {
         // On ne regarde que les deux premiers params. argv[1] et argv[2] correspondant aux pseudonymes
         if (strlen(argv[i]) > MAX_LENGHT_USERNAME) {
            fprintf(stderr, "Error : the maximum length of usernames is 30.\n");
            return 2;
         }
         // CHECK IF --PARAM IN ARGV[1] OR ARGV[2] SHOULD RISE AN ERROR OR BE CONSIDERED AS AN USERNAME
         else if (strcmp(argv[i], "--bot") == 0) {
            continue;
         } 
         else if (strcmp(argv[i], "--manuel") == 0) {
            continue;
         }
         else {
            for (size_t j = 0; j < strlen(argv[i]); j++) {
            // On parcourt chaque caractere j de argv[i] avec argv[i][j]
            // On appelle ispunct() pour verifier s'il y a un caractere de ponctuation dans chaque param.
            // size_t pour la variable j == type de retour de strlen() (long unsigned int)
               if (ispunct((unsigned char)argv[i][j])) {
                  fprintf(stderr, "Error : punctuation characters are forbidden for usernames.\n");
                  return 3;
               }
            }
         }  
      }
      if (argc == 4) {
         if (strcmp(argv[3], BOT_MODE) == 0) {
            *bot_mode_activated = true;
         }
         else if (strcmp(argv[3], MANUAL_MODE) == 0) {
            *manual_mode_activated = true;
         }
      }
      else if (argc == 5) {
         if (strcmp(argv[3], BOT_MODE) == 0 || strcmp(argv[4], BOT_MODE) == 0) {
            *bot_mode_activated = true;
         }
         if (strcmp(argv[3], MANUAL_MODE) == 0 || strcmp(argv[4], MANUAL_MODE) == 0) {
            *manual_mode_activated = true;
         }
      }
   }
   return 0;
}


void myHandler(int received_signal) {
   if (received_signal == SIGINT) {
      printf("The user pressed CTRL+C.\n");
      exit(0);
   }
}


int main(int argc, char* argv[]) {
   bool bot_mode_activated = false;
   bool manual_mode_activated = false;
   int result_checking = checkVadlidArgv(argc, argv, &bot_mode_activated, &manual_mode_activated);
   if (result_checking != 0) {
      return result_checking;
   }
   if (manual_mode_activated) {
      signal(SIGINT, myHandler);
   }
   return 0;
}
