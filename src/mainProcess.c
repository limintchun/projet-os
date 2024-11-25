// KAHOUACHE Ayman 000545130
// LI Min-Tchun 000590125
// PERERA GONZALEZ Maxence 000590023
// Novembre 2024
// INFO-F201 - Systeme d'exploitation
// Projet 1 de programmation systeme ; chat - edition processus & pipe
// But du programme : programme chat permettant a deux utilisateurs de discuter a l'aide de pipes nommes.


// MAIN PROCESSUS CHARGE D'ENVOYER DES MESSAGES


// DECLARATION DES INCLUDES
#include "mainProcess.h"
#include "signals.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>


// CODE
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
// ajouter une gestion d'une erreur
// enter a considerer

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
                    // diff EOF error
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