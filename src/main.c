// KAHOUACHE Ayman 000545130
// MIN-TCHUN Li 000590125
// PERERA GONZALEZ Maxence 000590023
// Novembre 2024
// INFO-F201 - Systeme d'exploitation
// Projet 1 de programmation systeme ; chat - edition processus & pipe
// But du programme : programme chat permettant a deux utilisateurs de discuter a l'aide de pipes nommes.


#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>


/////////////////////////////////////////////// READ ME ///////////////////////////////////////////////
// CamelCase for struct
// camelCase for function
// under_score for variable
// Pourquoi mettre exit(code de retour) ou return code de retour ?
// EXIT :
// Situation exceptionnelle ou erreur critique -> stoper tout le programme.
// Fonctions de nettoyage avant de quitter le programme.
// Erreur critique dans une autre fonction -> aucun interet de revenir dans le main.
// RETURN :
// Dans le main pour indiquer le code de sortie de fin du programme.
// Quitter une fonction avec un retour specifique et "utilisable" (il existe un flag ; demander au prof).


#define MAX_LENGHT_USERNAME 30
#define DEFAULT_BUFFER_SIZE 1000
#define BOT_MODE           "--bot"
#define MANUAL_MODE        "--manuel"


void printUsernames(const char* text, bool bot_mode, bool oneself);
void pipesCleaning(char* pipe_user1_user2, char* pipe_user2_user1);
void initializePipesNames(char* pipe_user1_user2, char* pipe_user2_user1, const char* user1, const char* user2);
void sigintMainPrssHandler(int received_signal);
void sigintSecondPrssHandler();
void parseUsernames(char* argv[], int argv_index, const char** user1, const char** user2, const char* special_name);
void checkParseArgv(int argc, char* argv[], bool* bot_mode, bool* manual_mode, const char** user1, const char** user2);


void printUsernames(const char* text, bool bot_mode, bool oneself) {
   if (bot_mode) {
      printf("%s", text);
   }
   else {
      // Affiche en rouge et souligne.
      if (oneself) {
         printf("\x1b[31m\x1B[4m%s (vous)\x1B[0m\x1b[0m : ", text);
      }
      else {
         printf("\x1b[31m\x1B[4m%s\x1B[0m\x1b[0m : ", text);
      }
   }
}


void pipesCleaning(char* pipe_user1_user2, char* pipe_user2_user1) {
   unlink(pipe_user1_user2);
   unlink(pipe_user2_user1);
   // printf("Pipes %s and %s have been deleted.\n", pipe_user1_user2, pipe_user2_user1);
}


void initializePipesNames(char* pipe_user1_user2, char* pipe_user2_user1, const char* user1, const char* user2) {
   // Pourquoi utiliser snprintf et non sprintf ?
   // Meilleure version de sprintf car on se limite avec une taille de buffer.
   // Pas avoir de depassements (buffer overflows).
   // Tronque la sortie (coupe les donnees) en cas de buffer overflows.
   snprintf(pipe_user1_user2, DEFAULT_BUFFER_SIZE, "/tmp/%s-%s.chat", user1, user2);
   snprintf(pipe_user2_user1, DEFAULT_BUFFER_SIZE, "/tmp/%s-%s.chat", user2, user1);
}


void sigintMainPrssHandler(int received_signal) {
   if (received_signal == SIGINT) {
      printf("The user pressed CTRL+C.\n");
      exit(0);
   }
}


void sigintSecondPrssHandler() {
   // Ne pas avoir une interruption par CTRL+C dans un second processus.
   // SIG_IGN = constante.
   // SIG_IGN indique au progranne d'ignorer le signal SIGINT (CTRL+C qui termine immediatement un programme par defaut).
   // SIGINT n'est donc pas transmis au processus.
   signal(SIGINT, SIG_IGN);
}


void parseUsernames(char* argv[], int argv_index, const char** user1, const char** user2, const char* special_name) {
   if (special_name != NULL) {
      if (argv_index == 1) {
         *user1 = special_name;
      }
      else {
         *user2 = special_name;
      }
   }
   else {
      if (argv_index == 1) {
         *user1 = argv[argv_index];
      }
      else {
         *user2 = argv[argv_index];
      }
   }
}


void checkParseArgv(int argc, char* argv[], bool* bot_mode, bool* manual_mode, const char** user1, const char** user2) {
   if (1 == argc)  {
      fprintf(stderr, "chat pseudo_utilisateur pseudo_destinataire [--bot] [--manuel]\n");
      exit(1);
   }
   else {
      for (int i = 1; i < 3; i++) {
         // On ne regarde que les deux premiers params. argv[1] et argv[2] correspondant aux pseudonymes
         if (strlen(argv[i]) > MAX_LENGHT_USERNAME) {
            fprintf(stderr, "Error : the maximum length of usernames is 30.\n");
            exit(2);
         }
         // CHECK IF --NAME OR NAME WITHOUT "--"
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
                  exit(3);
               }
            }
            parseUsernames(argv, i, user1, user2, NULL);
         }  
      }
      if (argc == 4) {
         if (strcmp(argv[3], BOT_MODE) == 0) {
            *bot_mode = true;
         }
         else if (strcmp(argv[3], MANUAL_MODE) == 0) {
            *manual_mode = true;
         }
      }
      else if (argc == 5) {
         if (strcmp(argv[3], BOT_MODE) == 0 || strcmp(argv[4], BOT_MODE) == 0) {
            *bot_mode = true;
         }
         if (strcmp(argv[3], MANUAL_MODE) == 0 || strcmp(argv[4], MANUAL_MODE) == 0) {
            *manual_mode = true;
         }
      }
   }
}


int main(int argc, char* argv[]) {
   bool bot_mode = false;
   bool manual_mode = false;

   const char* user1;
   const char* user2;

   checkParseArgv(argc, argv, &bot_mode, &manual_mode, &user1, &user2);

   char pipe_user1_user2[DEFAULT_BUFFER_SIZE];
   char pipe_user2_user1[DEFAULT_BUFFER_SIZE];

   initializePipesNames(pipe_user1_user2, pipe_user2_user1, user1, user2);
   if (mkfifo(pipe_user1_user2, 0666) == -1 || mkfifo(pipe_user2_user1, 0666) == -1) {
      if (errno != EEXIST) {
         perror("mkfifo() error ; pipe hasn't been created.\n");
         pipesCleaning(pipe_user1_user2, pipe_user2_user1);
         return 1;
      }
   }

   // signal(SIGINT,);

   // Evite de terminer directement le programme si un user ferme son programme.
   // Sans signal(SIGPIPE, SIG_IGN) :
   // user2 quitte le chat, le programme d'user1 (qui ecrit un message) se termine immediatement.
   // Avec signal(SIGPIPE, SIG_IGN) :
   // user2 quitte le chat, le write d'user1 indique une erreur (meilleure gestion des erreurs).
   // Erreur = "user1 a quitte le chat" != interruption brutale.
   // signal(SIGPIPE, SIG_IGN);

   pid_t pid = fork();
   if (pid < 0) {
      perror("fork() error ; an error during fork process occurred.\n");
      pipesCleaning(pipe_user1_user2, pipe_user2_user1);
      return 1;
   }
   else if (pid == 0) {
      // Second processus : lire sur le pipe nomme adequat les messages et les afficher.
      // Toujours ignorer les interruptions CTRL+C dans le second processus.
      int fd_user2_user1_read = open(pipe_user2_user1, O_RDONLY);
      if (fd_user2_user1_read == -1) {
         perror("An error occurred during the attempt to open the reading pipe.\n");
         pipesCleaning(pipe_user1_user2, pipe_user2_user1);
         return 1;
      }

      char received_message[DEFAULT_BUFFER_SIZE];
      ssize_t readed_char;
      while(1) {
         readed_char = read(fd_user2_user1_read, received_message, DEFAULT_BUFFER_SIZE-1);
         if (readed_char == -1) {
            perror("Impossible d'ouvrir le pipe pour lire.\n");
            close(fd_user2_user1_read);
            pipesCleaning(pipe_user1_user2, pipe_user2_user1);
            return 1;
         }
         else if (readed_char > 0) {
            // '\0' sert a eviter de lire dans la memoire au-dela de la taille du tableau.
            // printf peut donc savoir ou la chaine de caracteres se termine.
            received_message[readed_char] = '\0';
            // "\r\033[K" fait un retour a la ligne + supprime tout son contenu.
            printf("\r\033[K"); 
            printUsernames(user2, bot_mode, false);
            printf("%s\n", received_message);
            printUsernames(user1, bot_mode, true);
            fflush(stdout);
         }

      }
      close(fd_user2_user1_read);
   }
   else {
      // Processus d'origine : lire les messages et les transmettre sur le pipe nomme adequat.
      int fd_user1_user2_write = open(pipe_user1_user2, O_WRONLY);
      if (fd_user1_user2_write == -1) {
         perror("An error occurred during the attempt to open the writing pipe.\n");
         pipesCleaning(pipe_user1_user2, pipe_user2_user1);
         return 1;
      }     

      char sended_message[DEFAULT_BUFFER_SIZE];
      printUsernames(user1, bot_mode, true);
      while (fgets(sended_message, DEFAULT_BUFFER_SIZE, stdin) != NULL) {
         size_t len_message = strlen(sended_message);
         if (len_message > 0 && sended_message[len_message-1] == '\n') {
            sended_message[len_message-1] = '\0';
         }
         if (write(fd_user1_user2_write, sended_message, strlen(sended_message)) == -1) {
            perror("Impossible d'ouvrir le pipe pour ecrire.\n");
            close(fd_user1_user2_write);
            pipesCleaning(pipe_user1_user2, pipe_user2_user1);
            return 1;
         }
         printUsernames(user1, bot_mode, true);
         fflush(stdout);
      }
      close(fd_user1_user2_write);
   }
   pipesCleaning(pipe_user1_user2, pipe_user2_user1);
   return 0;
}
