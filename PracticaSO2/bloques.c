// bloques.h


#include <stdio.h>  //printf(), fprintf(), stderr, stdout, stdin
#include <fcntl.h> //O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h> //S_IRUSR, S_IWUSR
#include <stdlib.h>  //exit(), EXIT_SUCCESS, EXIT_FAILURE, atoi()
#include <unistd.h> // SEEK_SET, read(), write(), open(), close(), lseek()
#include <errno.h>  //errno
#include <string.h> // strerror()
#include "bloques.h"

#define BLOCKSIZE 1024 // bytes


#define EXITO 0 //para gestión errores
#define FALLO -1 //para gestión errores


#define BLACK   "\x1B[30m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\x1B[37m"
#define ORANGE  "\x1B[38;2;255;128;0m"
#define ROSE    "\x1B[38;2;255;151;203m"
#define LBLUE   "\x1B[38;2;53;149;240m"
#define LGREEN  "\x1B[38;2;17;245;120m"
#define GRAY    "\x1B[38;2;176;174;174m"
#define RESET   "\x1b[0m"


#define NEGRITA "\x1b[1m"

//FUNCIONES

int bmount(const char *camino);
int bumount();
int bwrite(unsigned int nbloque, const void *buf);
int bread(unsigned int nbloque, void *buf);

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
