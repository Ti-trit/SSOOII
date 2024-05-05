#include "directorios.h"

/**
 * Permite escribir el texto en un posición de un fichero (offset)
 * 
*/
int main(int argc, char const *argv[]){
    if(argc != 5){
        fprintf(stderr, RED "Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n" RESET);
        return FALLO;
    }

  
    const char *ruta = (char*)argv[2];
    const unsigned int offset = atoi(argv[4]);

    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, RED "mi_escribir.c: Error al montar el dispositivo virtual\n" RESET);
        return FALLO;
    }

    // Si es un directorio error, no se puede escribir en uno
    if ((ruta[strlen(ruta) - 1]) == '/') {
        fprintf(stderr, RED "mi_escribir.c: No se puede escribir en directorios.\n" RESET);
        return FALLO;
    }


    fprintf(stdout, "Longitud texto: %li\n", strlen(argv[3]));
    int bytes = mi_write(ruta, argv[3], offset, strlen(argv[3]));
    if (bytes < 0) {
        fprintf(stderr, RED "mi_escribir.c: Error en mi_write()\n"RESET);
        bytes = 0;
    }
    fprintf(stdout, "Bytes escritos: %i\n", bytes);

    if (bumount() == FALLO){
        fprintf(stderr, RED "mi_escribir.c: Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    return EXITO;
}