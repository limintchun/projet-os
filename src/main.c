// KAHOUACHE Ayman 000545130
// MIN-TCHUN Li 000590125
// PERERA GONZALEZ Maxence 000590023
// Novembre 2024
// INFO-F201 - Systeme d'exploitation
// Projet 1 de programmation systeme ; chat - edition processus & pipe
// But du programme : programme chat permettant a deux utilisateurs de discuter a l'aide de pipes nommes.


// DECLARATION DES INCLUDES
#include <stdio.h>
#include <string.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>


// DECLARATION DES DEFINES
#define MAX_LENGTH_USERNAME   30
#define DECONNEXION_CODE      0
#define ARGV_CODE             1
#define LENGTH_USERNAME_CODE  2  
#define PUNCTUATION_CODE      3
#define SIGINT_NOPIPE_CODE    4
#define UNKNOWN_MODE_CODE     5
#define FATAL_ERROR_CODE      6
#define KILL_PROCESS_CODE     7
#define PIPE_NAME_SIZE        256
#define SHARED_MEMORY_SIZE    4096
#define BANNED_PUNCTUATION    ".-/[]"
#define BOT_MODE              "--bot"
#define MANUAL_MODE           "--manuel"
#define PRESTIGE_MODE         "--prestige"


// STRUCTURE POUR CENTRALISER TOUTES LES INFORMATIONS/RESSOURCES UTILES AU PROGRAMME
typedef struct {
    // Informations concernant la configuration du chat ; noms, modes de lancement
    const char* user1;
    const char* user2;
    bool manual_mode;
    bool bot_mode;
    bool prestige_mode;
    // Informations utiles au moment de terminer le programme
    bool pipes_opened;
    pid_t second_process_pid;
    // Ressources partagees necessitant un clean ; pipes, memoire partagee, gestion dynamique des messages
    char pipe_user1_user2[PIPE_NAME_SIZE];
    char pipe_user2_user1[PIPE_NAME_SIZE];
    int shared_memory_segment_id;
    char* shared_memory;
    FILE* writing_stream;
    FILE* reading_stream;
    char* sending_buffer;
    size_t sending_buffer_size;
    char* receiving_buffer;
    size_t receiving_buffer_size;
} ChatData;


// DECLARATION DES VARIABLES GLOBALES
volatile sig_atomic_t sigint_catched = 0;
volatile sig_atomic_t sigpipe_catched = 0;
volatile sig_atomic_t sigusr_catched = 0;
volatile sig_atomic_t sigterm_catched = 0;


// DECLARATION DES PROTOTYPES
static void signalsHandler(int signal);
void initializeChatData(ChatData* chat_data);
void initializeSignalsHandler();
void ignoreSigint();
void initializePipes(ChatData* chat_data);
void initializeSharedMemory(ChatData* chat_data);
bool parseUsernames(char* username);
void parseArgv(int argc, char* argv[], ChatData* chat_data);
void displayUsernames(bool oneself, ChatData* chat_data);
void displayPendingMessages(ChatData* chat_data);
void storeMessageInSharedMemory(char* message, ChatData* chat_data);
void cleanSharedMemory(ChatData* chat_data);
void cleanPipes(ChatData* chat_data);
void cleanStreamsBuffers(ChatData* chat_data);
void sigintMonitor(ChatData* chat_data);
void sigtermMonitor(ChatData* chat_data);
void sigpipeMonitor(ChatData* chat_data);
void sigusrMonitor(ChatData* chat_data);
void signalsMonitor(ChatData* chat_data);
void terminateProgram(bool is_error, ChatData* chat_data);
void secondProcessHandler(ChatData* chat_data);
void mainProcessHandler(ChatData* chat_data);


// CODE
static void signalsHandler(int signal) {
    switch (signal) {
        case    SIGINT: sigint_catched = 1;
                break;
        case    SIGPIPE: sigpipe_catched = 1;
                break;
        case    SIGUSR1: sigusr_catched = 1;
                break;
        case    SIGTERM: sigterm_catched = 1;
                break;
        default: break;
    }
}


void initializeChatData(ChatData* chat_data) {
    chat_data->user1 = NULL;
    chat_data->user2 = NULL;
    chat_data->manual_mode = false;
    chat_data->bot_mode = false;
    chat_data->prestige_mode = false;

    chat_data->pipes_opened = false;
    chat_data->second_process_pid = -1;

    chat_data->shared_memory_segment_id = -1;
    chat_data->shared_memory = NULL;
    chat_data->writing_stream = NULL;
    chat_data->reading_stream = NULL;
    chat_data->sending_buffer = NULL;
    chat_data->sending_buffer_size = 0;
    chat_data->receiving_buffer = NULL;
    chat_data->receiving_buffer_size = 0;
}


void initializeSignalsHandler() {
    struct sigaction action;
    action.sa_handler = signalsHandler;
    action.sa_flags = 0; // Relance une fonction bloquante en cas de signal
    sigemptyset(&action.sa_mask);

    if (sigaction(SIGINT, &action, NULL) < 0) {
        perror("\nFailed to set handler for SIGINT");
    }
    if (sigaction(SIGPIPE, &action, NULL) < 0) {
        perror("\nFailed to set handler for SIGPIPE");
    }
    if (sigaction(SIGUSR1, &action, NULL) < 0) {
        perror("\nFailed to set handler for SIGUSR1");
    }
    if (sigaction(SIGTERM, &action, NULL) < 0) {
        perror("\nFailed to set handler for SIGTERM");  
    }
}


void ignoreSigint () {
    struct sigaction ignore_action;
    ignore_action.sa_handler = SIG_IGN;
    ignore_action.sa_flags = 0;
    sigemptyset(&ignore_action.sa_mask);

    if (sigaction(SIGINT, &ignore_action, NULL) < 0) {
        perror("\nFailed to set handler for ignoring SIGINT in second process");
    }
}


void initializePipes(ChatData* chat_data) {
    if (snprintf(chat_data->pipe_user1_user2, PIPE_NAME_SIZE, "/tmp/%s-%s.chat", 
                chat_data->user1, chat_data->user2) >= PIPE_NAME_SIZE) {
        fprintf(stderr, "Error ; pipe user1 to user2 name exceeds buffer size.\n");
        terminateProgram(true, chat_data);
    }
    if (snprintf(chat_data->pipe_user2_user1, PIPE_NAME_SIZE, "/tmp/%s-%s.chat", 
                chat_data->user2, chat_data->user1) >= PIPE_NAME_SIZE) {
        fprintf(stderr, "Error ; pipe user2 to user1 name exceeds buffer size.\n");
        terminateProgram(true, chat_data);
    }

    if (mkfifo(chat_data->pipe_user1_user2, 0666) == -1 && errno != EEXIST) {
        perror("\nmkfifo() error ; pipe user1 to user2 hasn't been created");
        terminateProgram(true, chat_data);
    }
    if (mkfifo(chat_data->pipe_user2_user1, 0666) == -1 && errno != EEXIST) {
        perror("\nmkfifo() error ; pipe user2 to user1 hasn't been created");
        terminateProgram(true, chat_data);
    }
}


void initializeSharedMemory(ChatData* chat_data) {
    chat_data->shared_memory_segment_id = shmget(IPC_PRIVATE, SHARED_MEMORY_SIZE, IPC_CREAT | 0666);
    if (chat_data->shared_memory_segment_id == -1) {
        perror("\nFailed to create shared memory");
        terminateProgram(true, chat_data);
    }
    chat_data->shared_memory = (char*)shmat(chat_data->shared_memory_segment_id, NULL, 0);
    if (chat_data->shared_memory == (void*)-1) {
        perror("\nFailed to attach shared memory");
        terminateProgram(true, chat_data);
    }
    memset(chat_data->shared_memory, 0, SHARED_MEMORY_SIZE);
}


bool parseUsernames(char* username) {
    if (strcmp(username, BOT_MODE) == 0 || strcmp(username, MANUAL_MODE) == 0 || strcmp(username, PRESTIGE_MODE) == 0) {
        return false;
    }
    else {
        for (size_t i = 0; i < strlen(username); i++) {
            if (strchr(BANNED_PUNCTUATION, username[i]) != NULL) {
                return true;
            }
        }
    }
    return false;
}


void parseArgv(int argc, char* argv[], ChatData* chat_data) {
    if (3 > argc)  {
        fprintf(stderr, "chat pseudo_utilisateur pseudo_destinataire [--bot] [--manuel]\n");
        exit(ARGV_CODE);
    }
    for (int i = 1; i < 3; i++) {
        if (strlen(argv[i]) > MAX_LENGTH_USERNAME) {
            fprintf(stderr, "Error ; the maximum length of usernames is 30.\n");
            exit(LENGTH_USERNAME_CODE);
        }
        else if (parseUsernames(argv[i])) {
            fprintf(stderr, "Error ; punctuation characters are forbidden for usernames.\n");
            exit(PUNCTUATION_CODE);
        }
    }
    chat_data->user1 = argv[1];
    chat_data->user2 = argv[2];

    if (3 < argc) {
        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], BOT_MODE) == 0) {
                chat_data->bot_mode = true;
            }
            else if (strcmp(argv[i], MANUAL_MODE) == 0) {
                chat_data->manual_mode = true;
            }
            else if (strcmp(argv[i], PRESTIGE_MODE) == 0) {
                chat_data->prestige_mode = true;
            }
            else {
                fprintf(stderr, "Error ; allowed modes are --bot, --manuel and --prestige.\n");
                exit(UNKNOWN_MODE_CODE);
            }
        }
    }
}


void displayUsernames(bool oneself, ChatData* chat_data) {
    if (chat_data->bot_mode && oneself) {
        printf("[%s] ", chat_data->user1);
    }
    else if (chat_data->bot_mode && !oneself) {
        printf("[%s] ", chat_data->user2);
    }
    else if (!chat_data->bot_mode && oneself) {
        printf("[\x1b[31m\x1B[4m%s\x1B[0m\x1b[0m] ", chat_data->user1);
    }
    else {
        printf("[\x1b[31m\x1B[4m%s\x1B[0m\x1b[0m] ", chat_data->user2);
    }
}


void displayPendingMessages(ChatData* chat_data) {
    if (strlen(chat_data->shared_memory) > 0) {
        bool new_message = true;
        for (size_t i = 0; i < strlen(chat_data->shared_memory); i++) {
            if (new_message) {
                displayUsernames(false, chat_data);
                new_message = false;
            }

            putchar(chat_data->shared_memory[i]);

            if (chat_data->shared_memory[i] == '\n') {
                new_message = true;
            }
        }
        fflush(stdout);
        memset(chat_data->shared_memory, 0, SHARED_MEMORY_SIZE);
    }
}


void storeMessageInSharedMemory(char* message, ChatData* chat_data) {
    if (strlen(message) + strlen(chat_data->shared_memory) + 1 > SHARED_MEMORY_SIZE) {
        displayPendingMessages(chat_data);
        displayUsernames(false, chat_data);
        printf("%s", message);
    }
    else {
        strncat(chat_data->shared_memory, message, SHARED_MEMORY_SIZE - strlen(chat_data->shared_memory) - 1);
    }
}


void cleanSharedMemory(ChatData* chat_data) {
    if (chat_data->shared_memory != NULL) {
        if (shmdt(chat_data->shared_memory) == -1) {
            perror("\nFailed to detach shared memory");
        }
        chat_data->shared_memory = NULL;
    }
    if (chat_data->shared_memory_segment_id != -1) {
        struct shmid_ds info;
        if (shmctl(chat_data->shared_memory_segment_id, IPC_STAT, &info) == -1) {
            perror("\nError ; shared memory segment to remove does not exist or is inaccessible");
        } 
        else {
            if (shmctl(chat_data->shared_memory_segment_id, IPC_RMID, NULL) == -1) {
                perror("\nFailed to remove shared memory");
            } 
        }
        chat_data->shared_memory_segment_id = -1;
    }
}


void cleanPipes(ChatData* chat_data) {
    if (access(chat_data->pipe_user1_user2, F_OK) == 0) {
        if (unlink(chat_data->pipe_user1_user2) == -1) {
            perror("\nFailed to remove the pipe user1 to user2");
        }
    }
    if (access(chat_data->pipe_user2_user1, F_OK) == 0) {
        if (unlink(chat_data->pipe_user2_user1) == -1) {
            perror("\nFailed to remove the pipe user2 to user1");
        }
    }
}


void cleanStreamsBuffers(ChatData* chat_data) {
    if (chat_data->writing_stream != NULL) {
        fclose(chat_data->writing_stream);
        chat_data->writing_stream = NULL;
    }
    if (chat_data->reading_stream != NULL) {
        fclose(chat_data->reading_stream);
        chat_data->reading_stream = NULL;
    }

    if (chat_data->sending_buffer != NULL) {
        free(chat_data->sending_buffer);
        chat_data->sending_buffer = NULL;
    }
    if (chat_data->receiving_buffer != NULL) {
        free(chat_data->receiving_buffer);
        chat_data->receiving_buffer = NULL;
    }
}


void sigintMonitor(ChatData* chat_data) {
    if (sigint_catched) {
        if (chat_data->prestige_mode) {
            printf("\nYou pressed CTRL+C.\n");
        }
        if (chat_data->manual_mode && chat_data->pipes_opened) {
            displayPendingMessages(chat_data);
            sigint_catched = 0;
        }
        else {
            if (chat_data->prestige_mode) {
                printf("Exiting the chat.\n");
            }
            terminateProgram(false, chat_data);
        }
    }
}


void sigtermMonitor(ChatData* chat_data) {
    if (sigterm_catched) {
        cleanStreamsBuffers(chat_data);
        exit(KILL_PROCESS_CODE);
    }
}


void sigpipeMonitor(ChatData* chat_data) {
    if (sigpipe_catched) {
        if (chat_data->prestige_mode) {
            printf("%s left the chat.\n", chat_data->user2);
            printf("Exiting the chat.\n");
        }
        terminateProgram(false, chat_data);
    }
}


void sigusrMonitor(ChatData* chat_data) {
    if (sigusr_catched) {
        terminateProgram(true, chat_data);
    }
}


void signalsMonitor(ChatData* chat_data) {
    sigintMonitor(chat_data);
    sigpipeMonitor(chat_data);
    sigusrMonitor(chat_data);
}


void terminateProgram(bool is_error, ChatData* chat_data) {
    // Kill le processus fils que s'il existe
    if (chat_data->second_process_pid > 0) {
        kill(chat_data->second_process_pid, SIGTERM);
        waitpid(chat_data->second_process_pid, NULL, 0);
    }

    // Clean de toutes les ressources
    cleanSharedMemory(chat_data);
    cleanPipes(chat_data);
    cleanStreamsBuffers(chat_data);

    // Terminaison avec le code adapte
    if (is_error) {
        exit(FATAL_ERROR_CODE);
    }
    else if (!chat_data->pipes_opened) {
        exit(SIGINT_NOPIPE_CODE);
    }
    else {
        exit(DECONNEXION_CODE);
    }
}


void secondProcessHandler(ChatData* chat_data) {
    // Partie pour lancer la communication (ouverture du pipe de lecture)
    int fd_user2_user1_read;
    do {
        sigtermMonitor(chat_data);
        fd_user2_user1_read = open(chat_data->pipe_user2_user1, O_RDONLY);
    } while (fd_user2_user1_read == -1 && errno == EINTR);

    if (fd_user2_user1_read == -1) {
        perror("\nFailed to open the user2 to user1 pipe (reading pipe)");
        kill(getppid(), SIGUSR1);
        while (true) {
            sigtermMonitor(chat_data);
        }
    }

    chat_data->reading_stream = fdopen(fd_user2_user1_read, "r");
    if (chat_data->reading_stream == NULL) {
        perror("Failed to convert file descriptor to stream for reading");
        close(fd_user2_user1_read); // Doit quand meme fermer le descripteur de fichier
        kill(getppid(), SIGUSR1);
        while (true) {
            sigtermMonitor(chat_data);
        }
    }

    chat_data->pipes_opened = true; // Pipes ouverts

    // Partie du chat
    size_t index = 0;

    do {
        sigtermMonitor(chat_data);

        int c = fgetc(chat_data->reading_stream); // Lecture caractere par caractere dans le flux de lecture
        if (c == EOF) { // Le flux est interrompu soit par une erreur, un signal du pere ou une deconnexion de user2
            // Verifier d'abord si le signal SIGTERM a ete recu depuis le pere
            sigtermMonitor(chat_data); 
            if (feof(chat_data->reading_stream)) {
                // Le flux est coupe car user2 a quitte le chat
                // Signaler au pere que user2 a quitte le chat
                // Le pere ne devrait pas avoir besoin d'ecrire un message de plus apres la fermeture pour catch SIGPIPE
                kill(getppid(), SIGPIPE);
                while (true) {
                    sigtermMonitor(chat_data);
                }
            } else if (ferror(chat_data->reading_stream)) {
                // En cas d'erreur de flux
                perror("\nFailed to read the stream");
                kill(getppid(), SIGUSR1);
                while (true) {
                    sigtermMonitor(chat_data);
                }
            }
        }

        // Augmentation dynamique de la taille du tampon
        // +1 pour le nouveau caractere lu
        // +1 pour '\0' indiquant la terminaison de la chaine en construction
        char* new_buffer = realloc(chat_data->receiving_buffer, chat_data->receiving_buffer_size + 2);
        if (new_buffer == NULL) {
            perror("\nFaild to allocate memory in the second process");
            kill(getppid(), SIGUSR1);
            while (true) {
                sigtermMonitor(chat_data);
            }
        }

        // Mise a jour du pointeur du tampon
        chat_data->receiving_buffer = new_buffer;

        // Ajout du caractere au tampon
        // On indique la prochaine position pour ecrire le prochain caractere
        // Mise a jour de sa taille pour "correspondre" au nouvel ajout
        chat_data->receiving_buffer[index++] = (char)c;
        chat_data->receiving_buffer_size++;

        if (c == '\n') {
            // Le enter permet de savoir que la fin du message est atteinte et donc on a un message complet
            // Le tampon termine par '\0' indiquant la fin de la chaine
            chat_data->receiving_buffer[index] = '\0';
            if (chat_data->manual_mode) {
                printf("\a");
                storeMessageInSharedMemory(chat_data->receiving_buffer, chat_data);
            }
            else {
                displayUsernames(false, chat_data);
                printf("%s", chat_data->receiving_buffer);
            }
            fflush(stdout);
            // On remet tout a zero pour preparer la prochaine gestion dynamique du prochain message
            free(chat_data->receiving_buffer); 
            chat_data->receiving_buffer = NULL;
            chat_data->receiving_buffer_size = 0;
            index = 0;
        }

        sigtermMonitor(chat_data);
    } while (true);
}


void mainProcessHandler(ChatData* chat_data) {
    // Partie pour lancer la communication (ouverture du pipe d'ecriture)
    int fd_user1_user2_write;
    do {
        signalsMonitor(chat_data);
        fd_user1_user2_write = open(chat_data->pipe_user1_user2, O_WRONLY);
    } while (fd_user1_user2_write == -1 && errno == EINTR);

    if (fd_user1_user2_write == -1) {
        perror("\nFailed to open the user1 to user2 pipe (writing pipe)");
        terminateProgram(true, chat_data);
    }

    chat_data->writing_stream = fdopen(fd_user1_user2_write, "w");
    if (chat_data->writing_stream == NULL) {
        perror("\nFailed to convert file descriptor to stream for writing");
        close(fd_user1_user2_write); // Doit quand meme fermer le descripteur de fichier
        terminateProgram(true, chat_data);
    }

    chat_data->pipes_opened = true; // Pipes ouverts

    // Partie du chat
    if (chat_data->prestige_mode) {
        printf("%s joined the chat.\n", chat_data->user2);
    }

    do {
        signalsMonitor(chat_data);

        // Lecture d'une ligne de texte introduite depuis le terminal
        ssize_t read_input = getline(&chat_data->sending_buffer, &chat_data->sending_buffer_size, stdin);

        // Un point de controle pour savoir si la lecture du terminal a ete interrompue par un signal ou CTRL+D
        if (read_input == -1) {
            if (feof(stdin)) {
                // C'est un EOF donc l'utilisateur a utilise CTRL+D indiquant qu'il ne va plus ecrire sur stdin
                if (chat_data->prestige_mode) {
                    printf("You pressed CTRL+D.\n");
                }
                terminateProgram(false, chat_data);
            }
            else if (errno == EINTR) {
                signalsMonitor(chat_data); // Verifie si l'interruption est en raison d'un signal
                // Si le signal (SIGINT en mode manuel) ne termine pas le programme
                // On nettoie le flux pour reprendre une execution normale
                clearerr(stdin);
            }
        }


        signalsMonitor(chat_data);

        // Verifie si seul '\n' a ete introduit afin qu'il soit ignorer
        if (read_input > 0) {
            if (read_input != 1 || chat_data->sending_buffer[0] != '\n') {
                if (!chat_data->bot_mode) {
                    displayUsernames(true, chat_data);
                    printf("%s", chat_data->sending_buffer);
                }

                if (chat_data->manual_mode) {
                    displayPendingMessages(chat_data);
                }

                for (size_t i = 0; i < (size_t)read_input; i++) {
                    if (fputc(chat_data->sending_buffer[i], chat_data->writing_stream) == EOF) {
                        signalsMonitor(chat_data); // Verifie si EOF est en raison d'un signal
                        perror("\nFailed to use stream for writing");
                        terminateProgram(true, chat_data);
                    }
                }
                fflush(chat_data->writing_stream);
            }
        }

        signalsMonitor(chat_data);
    } while (true);
}


int main(int argc, char* argv[]) {
    ChatData chat_data;
    initializeChatData(&chat_data);
    initializeSignalsHandler();

    parseArgv(argc, argv, &chat_data);

    initializePipes(&chat_data);

    if (chat_data.manual_mode) {
        initializeSharedMemory(&chat_data);
    }

    chat_data.second_process_pid = fork();
    if (chat_data.second_process_pid < 0) {
        perror("\nfork() error ; an error occurred during fork process");
        terminateProgram(true, &chat_data);
    }
    else if (chat_data.second_process_pid == 0) {
        ignoreSigint();
        secondProcessHandler(&chat_data);
    }
    else {
        mainProcessHandler(&chat_data);
    }
    return 0;
}
