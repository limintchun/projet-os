#ifndef SIGNALS_H
#define SIGNALS_H


// DECLARATION DES INCLUDES
#include "ressources.h"
#include "chatGestion.h"
#include <signal.h>


// DECLARATION DES VARIABLES GLOBALES POUR LES SIGNAUX
extern volatile sig_atomic_t sigint_catched;
extern volatile sig_atomic_t sigpipe_catched;
extern volatile sig_atomic_t sigusr_catched;
extern volatile sig_atomic_t sigterm_catched;


// DECLARATION DES PROTOTYPES
void initializeSignalsHandler();
void ignoreSigint();
void signalsHandler(int signal);
void sigintMonitor(ChatData* chat_data);
void sigtermMonitor(ChatData* chat_data);
void sigpipeMonitor(ChatData* chat_data);
void sigusrMonitor(ChatData* chat_data);
void signalsMonitor(ChatData* chat_data);


#endif // SIGNALS_H