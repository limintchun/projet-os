#include <stdio.h> //biblio standart
//processus
#include <sys/types.h> //PID
#include <unistd.h> //Fork(), exec(), pipes
#include <sys/wait.h> //Eventuel pour mettre en pause le processus
//Threads
#include <pthread.h>
//Strings ?
#include <string.h>

int chat(char pseudo_utilisateur[30],char pseudo_destinataire[30], param1, param2 ){
  if(param1 != NULL){
      TODO
      //cas --bot

  }else if(param2 != NULL){
      TODO
      //cas manuel
  }


}
int main(int argc, char* argv[]) {
  /* Tous les appels systèmes devront être placé dans main()
  Tandis que "chat" sera considéré comme processus et ne gérera que
  le comportement.

  */

   return 0;
}


