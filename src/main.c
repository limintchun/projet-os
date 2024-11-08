#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>



#define MAX_LENGHT_USERNAME 30
#define DEFAULT_BUFFER_SIZE 1000
#define BOT_MODE           "--bot"
#define MANUAL_MODE        "--manuel"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"



void initializePipesNames(char* pipe_user1_user2, char* pipe_user2_user1, const char* user1, const char* user2);
void myHandler(int received_signal);
void affichage(char username[], char message[], bool modificationActivate);
void parseUsernames(char* argv[], int argv_index, const char** user1, const char** user2, const char* special_name);
int checkParseArgv(int argc, char* argv[], bool* bot_mode, bool* manual_mode, const char** user1, const char** user2);
void parent(int write_fd);
void fils(int read_fd, char sender[]);


void initializePipesNames(char* pipe_user1_user2, char* pipe_user2_user1, const char* user1, const char* user2) {
   // Pourquoi utiliser snprintf et non sprintf ?
   // Meilleure version de sprintf car on se limite avec une taille de buffer.
   // Pas avoir de depassements (buffer overflows).
   // Tronque la sortie (coupe les donnees) en cas de buffer overflows.
   snprintf(pipe_user1_user2, DEFAULT_BUFFER_SIZE, "/tmp/%s-%s.chat", user1, user2);
   snprintf(pipe_user2_user1, DEFAULT_BUFFER_SIZE, "/tmp/%s-%s.chat", user2, user1);
}

void myHandler(int received_signal) {
   if (received_signal == SIGINT) {
      printf("The user pressed CTRL+C.\n");
      exit(0);
   }
}


void affichage(char username[], char message[], bool modificationActivate) {
  if (modificationActivate == true) {
    printf("[%s] : %s", username, message);
  }else{
     printf("[" ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET "] %s\n", username, message);
  }
}


void parseUsernames(char* argv[], int argv_index, const char** user1, const char** user2, const char* special_name) {
  if (special_name != NULL) {
    if (argv_index == 1) {
      *user1 = special_name;
    }else {
      *user2 = special_name;
    }
  }else {
      if (argv_index == 1) {
         *user1 = argv[argv_index];
      }else {
         *user2 = argv[argv_index];
      }
   }
}


int checkParseArgv(int argc, char* argv[], bool* bot_mode, bool* manual_mode, const char** user1, const char** user2) {
   if (1 == argc){
      fprintf(stderr, "chat pseudo_utilisateur pseudo_destinataire [--bot] [--manuel]\n");
      return 1;
   }else{
      for (int i = 1; i < 3; i++) {
         // On ne regarde que les deux premiers params. argv[1] et argv[2] correspondant aux pseudonymes
         if (strlen(argv[i]) > MAX_LENGHT_USERNAME) {
            fprintf(stderr, "Error : the maximum length of usernames is 30.\n");
            return 2;
         }
         // CHECK IF --PARAM IN ARGV[1] OR ARGV[2] SHOULD RISE AN ERROR OR BE CONSIDERED AS AN USERNAME
         else if (strcmp(argv[i], "--bot") == 0) {
            parseUsernames(argv, i, user1, user2, "Bot");
         }
         else if (strcmp(argv[i], "--manuel") == 0) {
            parseUsernames(argv, i, user1, user2, "Manuel");
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
            parseUsernames(argv, i, user1, user2, NULL);
         }
      } if (argc == 4) {
         if (strcmp(argv[3], BOT_MODE) == 0) {
            *bot_mode = true;

         }else if (strcmp(argv[3], MANUAL_MODE) == 0) {
            *manual_mode = true;
         }
      }else if (argc == 5) {
         if (strcmp(argv[3], BOT_MODE) == 0 || strcmp(argv[4], BOT_MODE) == 0) {
            *bot_mode = true;
         }
         if (strcmp(argv[3], MANUAL_MODE) == 0 || strcmp(argv[4], MANUAL_MODE) == 0) {
            *manual_mode = true;
         }
      }
   }
   return 0;
}

void parent(int write_fd) {
  printf("je suis le père\n");
  printf("vous pouvez débuter votre discussion\n");

  char buffer[256];
  while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
    write(write_fd, buffer, strlen(buffer) + 1);
  }//## eventuelle ;
}


void fils(int read_fd, char sender[]) {
  printf("je suis le fils \n");

  char buffer[256];
  while (read(read_fd, buffer, sizeof(buffer)) > 0) {
    affichage(sender, buffer, false);
  }

}

int main(int argc, char* argv[]){
  bool bot_mode = false;
  bool manual_mode = false;
  const char* user1;
  const char* user2;
  int result_checking = checkParseArgv(argc, argv, &bot_mode, &manual_mode, &user1, &user2);
    if (result_checking != 0){
      return result_checking;
    }

    if (manual_mode) {
      signal(SIGINT, myHandler);
    }

  char pipe_user1_user2[DEFAULT_BUFFER_SIZE]; // utilisé pour write
  char pipe_user2_user1[DEFAULT_BUFFER_SIZE]; // utilisé pour read
  // Sachant que l'on va lancer 2 fois le programme
  initializePipesNames(pipe_user1_user2, pipe_user2_user1, user1, user2);

  unlink(pipe_user1_user2);
  unlink(pipe_user2_user1);
  if(mkfifo(pipe_user1_user2, 0666) == -1 || mkfifo(pipe_user2_user1, 0666) == -1) {
    perror("mkfifo() error ; pipe hasn't been created.\n");
    return 1;
  }
  pid_t pid = fork();
  if(pid > 0){
    int fd_user1_user2_write = open(pipe_user1_user2, O_WRONLY);
    if (fd_user1_user2_write == -1) {
      perror("échec d'ouverture du pipe : ");
      unlink(fd_user1_user2_write);
      return 1;
    }
  parent(fd_user1_user2);
  close(fd_user1_user2);
  wait(NULL);
  unlink(user1);
 }else if (process == 0) { // process qui affiche ce qui est lu sur le pipe
     int fd_user1_user2_read = open(pipe_user2_user1, O_RDONLY); // same

     if (fd_user1_user2_read == -1) {
       perror("échec d'ouverture du pipe : ");
       unlink(pipe_user2_user1);
       return 1;
     }
  fils(fd_user1_user2_read)
  close(fd_user1_user2_read);
  unlink(user2);
  }else:
     perror("fork()");
     return 1;

  return 1;
}
