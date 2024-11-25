#ifndef CHAT_GESTION_H
#define CHAT_GESTION_H


// DECLARATION DES INCLUDES
#include "ressources.h"


// DECLARATION DES PROTOTYPES
bool parseUsernames(char* username);
void parseArgv(int argc, char* argv[], ChatData* chat_data);
void displayUsernames(bool oneself, ChatData* chat_data);
void displayPendingMessages(ChatData* chat_data);
void storeMessageInSharedMemory(char* message, ChatData* chat_data);


#endif // CHAT_GESTION_H