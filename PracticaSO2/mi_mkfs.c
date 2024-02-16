#include "bloques.h"
#include <stdio.h>
#include <string.h>


/**
 * Programa principal
 * Formatea el dispositivo virtual con el tamaño adecuado de bloques, nbloques. 
 * @param   argc numero de parametros en argv
 * @param   argv    cadena de comandos
 * @return  0 si el programa se ejecuto correctamente
*/
 

int main(int argc, char **argv){

if (argc!=3){
    fprintf(stderr, RED"Sintaxis incorrecta:./mi_mkfs <nombre_dispositivo> <nbloques>."RESET);
    return FALLO;
}
//obtener nombre de dispositivo
char * camino = argv[1];
//numero de bloques
int nbloques = atoi(argv[2]);

//montar el dispositivo virtual
  if (bmount(camino)<0){
    fprintf(stderr, RED "Se ha producido un error al montar el dispositivo");
    return FALLO;
  }

    unsigned char buf [nbloques];
    memset(buf, 0, nbloques);
  for (int i = 0; i<nbloques; i++){
   if ( bwrite(i, buf)<1){
    fprintf(stderr, RED"Error al escribir en el bloque %i"RESET, i);
    return FALLO;
   }

  }

  //desmontar el dispositivo virtual
  if (bmount(camino)<0){

    fprintf(stderr, RED"Error al desmontar el dispositivo" RESET);
    return FALLO;
  }

    


}
