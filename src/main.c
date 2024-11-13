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


#define MAX_LENGHT_USERNAME 30
#define DEFAULT_BUFFER_SIZE 1000
#define BOT_MODE           "--bot"
#define MANUAL_MODE        "--manuel"


typedef struct {
   char pipe_user1_user2[DEFAULT_BUFFER_SIZE];
   char pipe_user2_user1[DEFAULT_BUFFER_SIZE];
   const char* user1;
   const char* user2;
   bool manual_mode;
   bool bot_mode;
   bool reading_pipe_open;
   bool writing_pipe_open;
   bool pipes_created;
} ChatConfiguration;


void initializeChatConfiguration(ChatConfiguration* chat_configuration);
void parseUsernames(char* argv[], int argv_index, const char* special_name, ChatConfiguration* chat_configuration);
void checkParseArgv(int argc, char* argv[], ChatConfiguration* chat_configuration);
void initializeCreatePipes(ChatConfiguration* chat_configuration);
void printUsernames(bool oneself, ChatConfiguration* chat_configuration);
void pipesCleaning(ChatConfiguration* chat_configuration);
void sigpipeHandler(int received_signal);
void sigintMainProcessHandler(int received_signal);
void sigintSecondProcessHandler();
void mainProcessHandling(ChatConfiguration* chat_configuration);
void secondProcessHandling(ChatConfiguration* chat_configuration);
int main(int argc, char* argv[]);


// Variable globale pour acceder a la structure dans le gestionnaire de signal.
ChatConfiguration* global_chat_configuration = NULL;


void initializeChatConfiguration(ChatConfiguration* chat_configuration) {
   global_chat_configuration = chat_configuration;
   chat_configuration->user1 = NULL;
   chat_configuration->user2 = NULL;
   chat_configuration->manual_mode = false;
   chat_configuration->bot_mode = false;
   chat_configuration->reading_pipe_open = false;
   chat_configuration->writing_pipe_open = false;
   chat_configuration->pipes_created = true;
}


void parseUsernames(char* argv[], int argv_index, const char* special_name, ChatConfiguration* chat_configuration) {
   if (special_name != NULL) {
      if (argv_index == 1) {
         chat_configuration->user1 = special_name;
      }
      else {
         chat_configuration->user2 = special_name;
      }
   }
   else {
      if (argv_index == 1) {
         chat_configuration->user1 = argv[argv_index];
      }
      else {
         chat_configuration->user2 = argv[argv_index];
      }
   }
}


void checkParseArgv(int argc, char* argv[], ChatConfiguration* chat_configuration) {
   if (3 > argc)  {
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
            parseUsernames(argv, i, "Bot", chat_configuration);
         } 
         else if (strcmp(argv[i], "--manuel") == 0) {
            parseUsernames(argv, i, "Manuel", chat_configuration);
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
            parseUsernames(argv, i, NULL, chat_configuration);
         }  
      }
      if (argc == 4) {
         if (strcmp(argv[3], BOT_MODE) == 0) {
            chat_configuration->bot_mode = true;
         }
         else if (strcmp(argv[3], MANUAL_MODE) == 0) {
            chat_configuration->manual_mode = true;
         }
      }
      else if (argc == 5) {
         if (strcmp(argv[3], BOT_MODE) == 0 || strcmp(argv[4], BOT_MODE) == 0) {
            chat_configuration->bot_mode = true;
         }
         if (strcmp(argv[3], MANUAL_MODE) == 0 || strcmp(argv[4], MANUAL_MODE) == 0) {
            chat_configuration->manual_mode = true;
         }
      }
   }
}


void initializeCreatePipes(ChatConfiguration* chat_configuration) {
   // Pourquoi utiliser snprintf et non sprintf ?
   // Meilleure version de sprintf car on se limite avec une taille de buffer.
   // Pas avoir de depassements (buffer overflows).
   // Tronque la sortie (coupe les donnees) en cas de buffer overflows.
   snprintf(chat_configuration->pipe_user1_user2, DEFAULT_BUFFER_SIZE, "/tmp/%s-%s.chat", 
   chat_configuration->user1, chat_configuration->user2);
   snprintf(chat_configuration->pipe_user2_user1, DEFAULT_BUFFER_SIZE, "/tmp/%s-%s.chat", 
   chat_configuration->user2, chat_configuration->user1);

   if (mkfifo(chat_configuration->pipe_user1_user2, 0666) == -1 && errno != EEXIST) {
      perror("mkfifo() error ; pipe hasn't been created.\n");
      unlink(chat_configuration->pipe_user1_user2);
      exit(1);
   }
   if (mkfifo(chat_configuration->pipe_user2_user1, 0666) == -1 && errno != EEXIST) {
      perror("mkfifo() error ; pipe hasn't been created.\n");
      unlink(chat_configuration->pipe_user2_user1);
      exit(1);
   }
}


void printUsernames(bool oneself, ChatConfiguration* chat_configuration) {
   if (chat_configuration->bot_mode && oneself) {
      printf("%s", chat_configuration->user1);
   }
   else if (chat_configuration->bot_mode && !oneself) {
      printf("%s", chat_configuration->user2);
   }
   else {
      // Affiche en rouge et souligne.
      if (oneself) {
         printf("\x1b[31m\x1B[4m%s (vous)\x1B[0m\x1b[0m : ", chat_configuration->user1);
      }
      else {
         printf("\x1b[31m\x1B[4m%s\x1B[0m\x1b[0m : ", chat_configuration->user2);
      }
   }
}


void pipesCleaning(ChatConfiguration* chat_configuration) {
   if (chat_configuration->pipes_created) {
      // F_OK return 0 si le fichier existe.
      if (access(chat_configuration->pipe_user1_user2, F_OK) == 0) {
         unlink(chat_configuration->pipe_user1_user2);
      }
      if (access(chat_configuration->pipe_user2_user1, F_OK) == 0) {
         unlink(chat_configuration->pipe_user2_user1);
      }
   }
}


void sigpipeHandler(int received_signal) {
   if (received_signal == SIGPIPE) {
      fprintf(stderr, "%s has left the chat.\n", global_chat_configuration->user2);
      pipesCleaning(global_chat_configuration);
      exit(0);
   }
}


void sigintMainProcessHandler(int received_signal) {
   if (received_signal == SIGINT) {
      fprintf(stderr, "You pressed CTRL+C.\n");
      pipesCleaning(global_chat_configuration);

      int exit_code = (!global_chat_configuration->reading_pipe_open && 
      !global_chat_configuration->writing_pipe_open) ? 4 : 0;
      
      kill(0, SIGTERM); 
      exit(exit_code);
   }
}


void sigintSecondProcessHandler() {
   // Ne pas avoir une interruption par CTRL+C dans un second processus.
   // SIG_IGN = constante.
   // SIG_IGN indique au progranne d'ignorer le signal SIGINT (CTRL+C qui termine immediatement un programme par defaut).
   // SIGINT n'est donc pas transmis au processus.
   signal(SIGINT, SIG_IGN);
}


void mainProcessHandling(ChatConfiguration* chat_configuration) {
   int fd_user1_user2_write = open(chat_configuration->pipe_user1_user2, O_WRONLY);
   if (fd_user1_user2_write == -1) {
      perror("An error occurred during the attempt to open the writing pipe.\n");
      pipesCleaning(chat_configuration);
      exit(1);
   }
   chat_configuration->writing_pipe_open = true; 

   char sended_message[DEFAULT_BUFFER_SIZE];
   printUsernames(true, chat_configuration);
   while (fgets(sended_message, DEFAULT_BUFFER_SIZE, stdin) != NULL) {
      size_t len_message = strlen(sended_message);
      if (len_message > 0 && sended_message[len_message-1] == '\n') {
         sended_message[len_message-1] = '\0';
      }
      if (write(fd_user1_user2_write, sended_message, strlen(sended_message)) == -1) {
         perror("Impossible d'ouvrir le pipe pour ecrire.\n");
         close(fd_user1_user2_write);
         pipesCleaning(chat_configuration);
         exit(1);
      }
      printUsernames(true, chat_configuration);
      fflush(stdout);
   }
   close(fd_user1_user2_write);
   chat_configuration->writing_pipe_open = false;   
}


void secondProcessHandling(ChatConfiguration* chat_configuration) {
   int fd_user2_user1_read = open(chat_configuration->pipe_user2_user1, O_RDONLY);
   if (fd_user2_user1_read == -1) {
      perror("An error occurred during the attempt to open the reading pipe.\n");
      pipesCleaning(chat_configuration);
      exit(1);
   }
   chat_configuration->reading_pipe_open = true;

   char received_message[DEFAULT_BUFFER_SIZE];
   ssize_t readed_char;
   while(1) {
      readed_char = read(fd_user2_user1_read, received_message, DEFAULT_BUFFER_SIZE-1);
      if (readed_char == -1) {
         perror("Impossible d'ouvrir le pipe pour lire.\n");
         close(fd_user2_user1_read);
         pipesCleaning(chat_configuration);
         exit(1);
      }
      else if (readed_char > 0) {
         // '\0' sert a eviter de lire dans la memoire au-dela de la taille du tableau.
         // printf peut donc savoir ou la chaine de caracteres se termine.
         received_message[readed_char] = '\0';
         // "\r\033[K" fait un retour a la ligne + supprime tout son contenu.
         printf("\r\033[K"); 
         printUsernames(false, chat_configuration);
         printf("%s\n", received_message);
         printUsernames(true, chat_configuration);
         fflush(stdout);
      }

   }
   close(fd_user2_user1_read);
   chat_configuration->reading_pipe_open = false;
}


int main(int argc, char* argv[]) {
   ChatConfiguration chat_configuration;
   initializeChatConfiguration(&chat_configuration);

   checkParseArgv(argc, argv, &chat_configuration);

   signal(SIGINT, sigintMainProcessHandler);

   initializeCreatePipes(&chat_configuration);

   signal(SIGPIPE, sigpipeHandler);

   pid_t pid = fork();
   if (pid < 0) {
      perror("fork() error ; an error during fork process occurred.\n");
      pipesCleaning(&chat_configuration);
      return 1;
   }
   else if (pid == 0) {
      // Second processus : lire sur le pipe nomme adequat les messages et les afficher.
      // Toujours ignorer les interruptions CTRL+C dans le second processus.
      sigintSecondProcessHandler();
      secondProcessHandling(&chat_configuration);
   }
   else {
      // Processus d'origine : lire les messages et les transmettre sur le pipe nomme adequat.
      mainProcessHandling(&chat_configuration);
   }
   pipesCleaning(&chat_configuration);
   return 0;
}
