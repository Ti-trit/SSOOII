/*
Autores: Khaoula Ikkene, Tomás Gallardo Rago, Francesc Gayá Piña  
Grupo: AntiLinux

*/
#include "bloques.h"




//Variables globales
static int descriptor = 0;

/**
 * Monta el dispositivo virtual, abre el fichero
 * @param camino      nombre del fichero
 * @return           el descriptor o -1 en caso de error
 * 
*/
int bmount(const char *camino){
    // Configurar umask del proceso para no restringir permisos
    umask(000);
    
    // abrir el archivo con permisos de lectura y escritura,
    // creandolo si no existe
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);

    //Verifica si hubo un error al abrir el archivo
    if (descriptor == -1) {
        fprintf(stderr, RED "Error al abrir el fichero.\n");
        return FALLO;
    }
    //Si todo va bien, retorn el descriptor del archivos
    return descriptor;
}

/**
 * Desmonta el dispositivo virtual
 * @return           0 si lo ha cerrado correctamente. -1 en otro caso
*/
int bumount() {
    //Si no se ha cerrado devuelve -1
    if (close(descriptor) == -1) {
        return FALLO;
    }
    //Si se ha cerrado devuelve 0
    return EXITO;
}

/**
 * Escribe 1 bloque en el dispositivo virtual, en el bloque físico (nbloque).
 * @param nbloque     posición virtual del bloque
 * @param buf         buffer de memoria para escribir 
 * @return            número de bytes escritos 
*/
int bwrite(unsigned int nbloque, const void *buf){
int pos = nbloque*BLOCKSIZE;//poisicon del bloque

if (lseek(descriptor, pos, SEEK_SET)<0){   //posicionar el puntero dentro del bloque
    fprintf(stderr, RED NEGRITA"Error al posicionar el puntero en bwrite().\n"RESET);
    return FALLO;
}
//Escribir en el bloque
int nbytes = write(descriptor,buf,BLOCKSIZE);
if (nbytes<0){
    fprintf(stderr,RED"Error al escribir el bloque %i.\n" RESET, nbloque);
        return FALLO;
}else{
    //número de bytes escritos en el fichero
    return nbytes;
}

}

/**
 * Lee 1 bloque del dispositivo virtual, correspondiente al físico nbloque.
 * @param nbloque     posición virtual del bloque
 * @param buf         buffer de memoria cuyo contenido intentaremos leer
 * @return             número de bytes leedos.FALLO en caso de error 
*/
int bread(unsigned int nbloque, void*buf){
    int pos = nbloque*BLOCKSIZE;// Posición inicial

    if (lseek(descriptor, pos, SEEK_SET)<0){
        fprintf(stderr, RED NEGRITA"Error al posicionar el puntero en bread().\n"RESET);
        return FALLO; // Error al posicionar el puntero
    }

    // Lee el contenido del bloque
    int nbytes = read(descriptor,buf,BLOCKSIZE);
    if(nbytes>=0){
        return nbytes; // Devuelve nbytes == BLOCKSIZE
    }else{
        fprintf(stderr, RED NEGRITA"bread: Error al leer en el bloque %i.\n"RESET, nbloque);
        return FALLO; // Salta un error en el bloque "nbloque"
    }
    return EXITO;
}
