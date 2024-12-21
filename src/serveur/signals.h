#ifndef SIGNALS_H
#define SIGNALS_H


// DECLARATION DES INCLUDES
#include <signal.h>


// DECLARATION DES VARIABLES GLOBALES POUR LES SIGNAUX
extern volatile sig_atomic_t sigint_catched;
extern volatile sig_atomic_t sigpipe_catched;


// DECLARATION DES PROTOTYPES
void initializeSignalsHandler();
void sigintSock(int sig);
void sigpipeSock(int sig);
void signalsHandler(int signal);


#endif // SIGNALS_H
