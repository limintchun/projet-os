// KAHOUACHE Ayman 000545130
// LI Min-Tchun 000590125
// PERERA GONZALEZ Maxence 000590023
// Novembre 2024
// INFO-F201 - Systeme d'exploitation
// Projet 1 de programmation systeme ; chat - edition processus & pipe
// But du programme : programme chat permettant a deux utilisateurs de discuter a l'aide de pipes nommes.


// ENTREE DU PROGRAMME


// DECLARATION DES INCLUDES
#include "signals.h"
#include "ressources.h"
#include "chatGestion.h"
#include "secondProcess.h"
#include "mainProcess.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>


// CODE - POINT D'ENTREE DU PROGRAMME
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
