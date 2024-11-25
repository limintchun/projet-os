#ifndef RESSOURCES_H
#define RESSOURCES_H


// DECLARATION DES INCLUDES
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>


// DECLARATION DES DEFINES
#define MAX_LENGTH_USERNAME   30
#define PIPE_NAME_SIZE        256
#define SHARED_MEMORY_SIZE    4096
#define BANNED_PUNCTUATION    ".-/[]"
#define BOT_MODE              "--bot"
#define MANUAL_MODE           "--manuel"
#define PRESTIGE_MODE         "--prestige"


// ENUM POUR LES CODES DE RETOUR
typedef enum {
    DECONNEXION_CODE = 0,
    ARGV_CODE,
    LENGTH_USERNAME_CODE,
    PUNCTUATION_CODE,
    SIGINT_NOPIPE_CODE,
    UNKNOWN_MODE_CODE,
    FATAL_ERROR_CODE,
    KILL_PROCESS_CODE
} ChatExitCode;


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


// DECLARATION DES PROTOTYPES
void initializeChatData(ChatData* chat_data);
void initializePipes(ChatData* chat_data);
void initializeSharedMemory(ChatData* chat_data);
void cleanSharedMemory(ChatData* chat_data);
void cleanPipes(ChatData* chat_data);
void cleanStreamsBuffers(ChatData* chat_data);
void terminateProgram(bool is_error, ChatData* chat_data);


#endif // RESSOURCES_H