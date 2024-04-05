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

    int offset = 0;
    char buffer_texto[tambuffer];
    int numBytesLeidos = 0;
    int leidos = 0;
    memset(buffer_texto,0,tambuffer);
        //obtener los bytes leedos
    leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer)
    if ( leidos < 0){
        fprintf(stderr, RED"Se ha producido un error al leer el bloque\n"RESET);
        return FALLO;
    }
    //numBytesLeidos += leidos;
    
    while (leidos > 0) {
        numBytesLeidos += leidos;
        write(1, buffer_texto, leidos);
        memset(buffer_texto,0,tambuffer);
        offset += tambuffer;
        if (leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer) < 0){
            fprintf(stderr, RED"Se ha producido un error al leer el bloque\n"RESET);
            return FALLO;
        }
    }

   struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, "Error main leer_inodo");
        return FALLO;
    }
        char string[128];
        sprintf(string, "\ntotal_leidos: %d\ntamEnBytesLog: %d\n", bytesLeidos, inodo.tamEnBytesLog);
        write(2, string, strlen(string));
  
    if (bumount() == FALLO) {
        fprintf(stderr, "Error main bumount");
        return FALLO;
    }
return EXITO;
}
