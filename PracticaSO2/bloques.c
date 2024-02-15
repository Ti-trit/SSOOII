
#include "bloques.h"




//Variables globales
static int descriptor = 0;


/**
 * Escribe 1 bloque en el dispositivo virtual, en el bloque físico (nbloque)
 * @par nbloque     posición virtual del bloque
 * @par buf         buffer de memoria cuyo contenido escribiremos en el fichero
 * @return  numero de bytes escrtitos
*/
int bwrite(unsigned int nbloque, const void *buf){
int pos = nbloque*BLOCKSIZE;//poision de bloque

lssek(descriptor, pos, SEEK_SET);   //posicionar el puntero dentro del bloque
int nbytes = write(descriptor,buf,BLOCKSIZE);//escribir en el bloque
if (nbytes<0){
    fprintf(stderr,RED"Error al escribir" RESET);
        return FALLO;
}else{
    //numero de bytes escritos en el fichero
    return nbytes;
}

}
