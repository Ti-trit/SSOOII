
#include "bloques.h"




//Variables globales
static int descriptor = 0;

/**
 * Monta el dispositivo virtual, abre el fichero
 * @par camino      nombre del fichero
 * @return          devuelve -1 si ha habido error, o el descriptor si ha ido bien
*/
int bmount(const char *camino){
    //Intenta abrir el archivo con permisos de lectura y escritura, creandolo si no existe
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);

    //Verifica si hubo un error al abrir el archivo
    if (descriptor == -1) {
        return -1;
    }
    //Si todo va bien, retorn el descriptor del archivo
    return descriptor;
}

/**
 * Desmonta el dispositivo virtual
 * @return          devuelve 0 si lo ha cerrado correctamente o -1 en caso contrario
*/
int bumount() {
    //Si no se ha cerrado devuelve -1
    if (close(descriptor) == -1) {
        return -1;
    }
    //Si se ha cerrado devuelve 0
    return 0;
}

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
