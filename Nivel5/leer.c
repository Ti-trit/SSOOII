#include "ficheros.h"

#define tambuffer 1500 //Tamaño del buffer de lectura

int main(int argc, int **argv) {

    if (argc=!3){
        fprintf(stderr, RED "Sintaxis: permitir <nombre_dispositivo> <ninodo>\n"RESET);
        return FALLO;
    }

    unsigned char dispositivo = argv[1];
    int ninodo = atoi(argv[2]);
    
    //montar el dispositivo

    if (bmount(dispositivo)==FALLO){
          fprintf(stderr, RED "Se ha producido un error al montar el dispositivo.\n"RESET);
            return FALLO;
    }

    memset(buffer_texto,0,tambuffer);
    int offset = 0;
    int numBytesLeidos = 0;
    int leidos = 0;

    if (leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer) < 0){
        fprintf(stderr, RED"Se ha producido un error al leer el bloque\n"RESET);
        return FALLO;
    }
    numBytesLeidos += leidos;
    
    while (leidos > 0) {
        write(1, buffer_texto, leidos);
        memset(buffer_texto,0,tambuffer);

        offset += tambuffer;
        if (leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer) < 0){
            fprintf(stderr, RED"Se ha producido un error al leer el bloque\n"RESET);
            return FALLO;
        }
        numBytesLeidos += leidos;
    }

    char string[128];
    sprintf(string, "bytes leídos %d\n", numBytesLeidos);
    write(2, string, strlen(string));

}