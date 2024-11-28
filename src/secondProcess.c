// KAHOUACHE Ayman 000545130
// LI Min-Tchun 000590125
// PERERA GONZALEZ Maxence 000590023
// Novembre 2024
// INFO-F201 - Systeme d'exploitation
// Projet 1 de programmation systeme ; chat - edition processus & pipe
// But du programme : programme chat permettant a deux utilisateurs de discuter a l'aide de pipes nommes.


// SECOND PROCESSUS CHARGE DE RECEVOIR LES MESSAGES


// DECLARATION DES INCLUDES
#include "secondProcess.h"
#include "signals.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>


// CODE
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
        sigtermWaitAndClean(chat_data);
    }

    // Convertir le file descriptor en FILE* stream
    chat_data->reading_stream = fdopen(fd_user2_user1_read, "r");
    if (chat_data->reading_stream == NULL) {
        perror("Failed to convert file descriptor to stream for reading");
        close(fd_user2_user1_read); // Doit quand meme fermer le descripteur de fichier
        kill(getppid(), SIGUSR1);
        sigtermWaitAndClean(chat_data);
    }

    chat_data->pipes_opened = true; // Pipes ouverts

    // Partie du chat
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
                sigtermWaitAndClean(chat_data);
            } 
            else if (ferror(chat_data->reading_stream)) {
                // En cas d'erreur du flux
                perror("\nFailed to read in the stream in the second process");
                kill(getppid(), SIGUSR1);
                sigtermWaitAndClean(chat_data);
            }
        }

        // Augmentation dynamique de la taille du tampon pour stocker le caractere lu
        // +1 pour le nouveau caractere lu
        // +1 pour '\0' indiquant la terminaison de la chaine en construction
        char* new_buffer = realloc(chat_data->receiving_buffer, chat_data->receiving_buffer_size + 2);
        if (new_buffer == NULL) {
            perror("\nFaild to allocate memory in the second process");
            kill(getppid(), SIGUSR1);
            sigtermWaitAndClean(chat_data);
        }

        // Mise a jour du pointeur du tampon
        chat_data->receiving_buffer = new_buffer;

        // Ajout du caractere au tampon
        // Mise a jour de sa taille pour "correspondre" au nouvel ajout
        chat_data->receiving_buffer[chat_data->receiving_buffer_size] = (char)c;
        chat_data->receiving_buffer_size++;

        // \n permet de savoir qu'on a un message complet
        if (c == '\n') {
            // Le tampon termine par '\0' indiquant la fin de la chaine
            chat_data->receiving_buffer[chat_data->receiving_buffer_size] = '\0';
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
        }

        sigtermMonitor(chat_data);
    } while (true);
}
