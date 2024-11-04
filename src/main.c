#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>



int main(int argc, char* argv[]) {
   if (1 < argc) {
      for (int i = 1; i < 3; i++) {
         // On ne regarde que les deux premiers params. argv[1] et argv[2] correspondant aux pseudonymes
         for (size_t j = 0; j < strlen(argv[i]); j++) {
            // On parcourt chaque caractere j de argv[i] avec argv[i][j]
            // On appelle ispunct() pour verifier s'il y a un caractere de ponctuation dans chaque param.
            // size_t pour la variable j == type de retour de strlen() (long unsigned int)
            if (ispunct((unsigned char)argv[i][j])) {
               fprintf(stderr, "Error : punctuation characters are forbidden.");
               return 3;
            }
         }
      }
   }
   return 0;
}
