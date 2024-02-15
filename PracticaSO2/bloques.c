
#include "bloques.h"




//Variables globales
static int descriptor = 0;


/**
 * Escribe 1 bloque en el dispositivo virtual, en el bloque físico (nbloque).
 * @param nbloque     posición virtual del bloque
 * @param buf         buffer de memoria cuyo contenido escribiremos en el fichero
 * @return  numero de bytes escritos 
*/
int bwrite(unsigned int nbloque, const void *buf){
int pos = nbloque*BLOCKSIZE;//poisicon del bloque

if (lssek(descriptor, pos, SEEK_SET)<0){   //posicionar el puntero dentro del bloque
    fprintf(stderr, RED NEGRITA"Error al posicionar el puntero.\n"RESET);
}
//Escribir en el bloque
int nbytes = write(descriptor,buf,BLOCKSIZE);
if (nbytes<0){
    fprintf(stderr,RED"Error al escribir el bloque %i.\n" RESET, nbloque);
        return FALLO;
}else{
    //numero de bytes escritos en el fichero
    return nbytes;
}

}
