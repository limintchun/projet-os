// KAHOUACHE Ayman 000545130
// LI Min-Tchun 000590125
// PERERA GONZALEZ Maxence 000590023
// Novembre 2024
// INFO-F201 - Systeme d'exploitation
// Projet 1 de programmation systeme ; chat - edition processus & pipe
// But du programme : programme chat permettant a deux utilisateurs de discuter a l'aide de pipes nommes.


// INITIALISATION DES SIGNAUX - GESTION DES SIGNAUX


// DECLARATION DES INCLUDES
#include "signals.h"
#include <stdatomic.h>
#include <signal.h>


// DECLARATION DES VARIABLES GLOBALES POUR LES SIGNAUX
volatile sig_atomic_t sigint_catched = 0;
volatile sig_atomic_t sigpipe_catched = 0;
volatile sig_atomic_t sigusr_catched = 0;
volatile sig_atomic_t sigterm_catched = 0;


// CODE
void initializeSignalsHandler() {
    struct sigaction action;
    action.sa_handler = signalsHandler;
    action.sa_flags = 0;
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


void signalsHandler(int signal) {
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