// KAHOUACHE Ayman 000545130
// LI Min-Tchun 000590125
// PERERA GONZALEZ Maxence 000590023
// Novembre 2024
// INFO-F201 - Systeme d'exploitation
// Projet 1 de programmation systeme ; chat - edition processus & pipe
// But du programme : programme chat permettant a deux utilisateurs de discuter a l'aide de pipes nommes.


// VERIFICATION DES NOMS/ARGV - AFFICHAGE A L'UTILISATEUR DES MESSAGES - STOCKAGE DES MESSAGES EN MEMOIRE PARTAGEE


// DECLARATION DES INCLUDES
#include "chatGestion.h"
#include <stdio.h>
#include <signal.h>
#include <string.h>


// CODE
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