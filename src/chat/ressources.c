// KAHOUACHE Ayman 000545130
// LI Min-Tchun 000590125
// PERERA GONZALEZ Maxence 000590023
// Novembre 2024
// INFO-F201 - Systeme d'exploitation
// Projet 1 de programmation systeme ; chat - edition processus & pipe
// But du programme : programme chat permettant a deux utilisateurs de discuter a l'aide de pipes nommes.


// RESSOURCES UTILES AUX PROGRAMMES - PIPES - MEMOIRE PARTAGEE - NETTOYAGE DES RESSOURCES - TERMINAISON DU PROGRAMME


// DECLARATION DES INCLUDES
#include "ressources.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <sys/ipc.h> 
#include <sys/mman.h>
#include <sys/wait.h>
#include <signal.h>


// CODE
void initializeChatData(ChatData* chat_data) {
    chat_data->user1 = NULL;
    chat_data->manual_mode = false;
    chat_data->bot_mode = false;
    chat_data->prestige_mode = false;

    chat_data->pipes_opened = false;
    chat_data->second_process_pid = -1;

    chat_data->writing_stream = NULL;
    chat_data->reading_stream = NULL;
    chat_data->shared_memory = NULL;
    chat_data->sending_buffer = NULL;
    chat_data->sending_buffer_size = 0;
    chat_data->receiving_buffer = NULL;
    chat_data->receiving_buffer_size = 0;
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
    // void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
    // NULL pour selection automatique d'une adresse pour le segment
    // int prot : PROT_READ | PROT_WRITE ; autorisations pour lire et ecrire dans la memoire partagee
    // int flags : MAP_SHARED ; chaque processus peut voir les modifications faites par les autres
    // int falgs : MAP_ANONYMOUS ; memoire partagee uniquement entre les processus sans utiliser un fichier
    // int fd : -1 car aucun fichier a mapper
    // off_t offset : offset mais ignore car il n'y a pas de fichier a mapper
    chat_data->shared_memory = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (chat_data->shared_memory == MAP_FAILED) {
        perror("\nFailed to map (create) shared memory");
        terminateProgram(true, chat_data);
    }
    
    // Initialiser la memoire partagee vide
    memset(chat_data->shared_memory, 0, SHARED_MEMORY_SIZE);
}


void cleanSharedMemory(ChatData* chat_data) {
    if (chat_data->shared_memory != NULL) {
        if (munmap(chat_data->shared_memory, SHARED_MEMORY_SIZE) == -1) {
           perror("\nFailed to unmap (remove) shared memory");
        }
        chat_data->shared_memory = NULL;
    }
}


// Pas besoin de clean un socket --> bidirictionnel

//void clean_socket(ChatData* chat_data) {
  //  if (access(chat_data->pipe_user1_user2, F_OK) == 0) {
    //    if (unlink(chat_data->pipe_user1_user2) == -1) {
      //      perror("\nFailed to remove the pipe user1 to user2");
        //}
   // }
    //if (access(chat_data->pipe_user2_user1, F_OK) == 0) {
      //  if (unlink(chat_data->pipe_user2_user1) == -1) {
        //    perror("\nFailed to remove the pipe user2 to user1");
        //}
   // }
//}


// sera utilise avec le mutex
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


// remplacer procees_pid par un thread
void terminateProgram(bool is_error, ChatData* chat_data) {
    // Kill le processus fils que s'il existe
    if (chat_data->second_process_pid > 0) {
        kill(chat_data->second_process_pid, SIGTERM);
        waitpid(chat_data->second_process_pid, NULL, 0);
    }

    // Clean de toutes les ressources
    cleanSharedMemory(chat_data);
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
