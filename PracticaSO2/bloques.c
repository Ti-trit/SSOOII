
#include "bloques.h"




//Variables globales
static int descriptor = 0;

/**
 * Monta el dispositivo virtual, abre el fichero
 * @param camino      nombre del fichero
 * @return          devuelve -1 si ha habido error, o el descriptor si ha ido bien
*/
int bmount(const char *camino){
    //Intenta abrir el archivo con permisos de lectura y escritura, creandolo si no existe
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);

    //Verifica si hubo un error al abrir el archivo
    if (descriptor == -1) {
        fprintf(stderr, RED "Error al abrir el fichero.\n");
        return FALLO;
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
        return FALLO;
    }
    //Si se ha cerrado devuelve 0
    return EXITO;
}

int bread(unsigned int nbloque, void*buf){
    int pos = nbloque*BLOCKSIZE;// Posición inicial.

    if (Iseek(descriptor, pos, SEEK_SET)<0){
        fprint(stderr, RED NEGRITA"Error al posicionar el puntero.\n"RESET);
        return FALLO; // Error al posicionar el puntero.
    }

    // Lee el contenido del bloque.
    int nbytes = read(descriptor,buf,BLOCKSIZE);
    if(nbytes>=0){
        return nbytes; // Devuelve nbytes == BLOCKSIZE.
    }else{
        fprint(stderr, RED NEGRITA"Error al leer en el bloque %i.\n"RESET, nbloque);
        return FALLO; // Salta un error en el bloque "nbloque".
    }
}
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
