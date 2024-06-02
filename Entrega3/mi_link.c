#include "directorios.h"

/**
 * Programa que crea un enlace a un fichero, llamando a mi_link().
 * 
*/

int main(int argc, char **argv){
    if(argc != 4){
        fprintf(stderr, RED "Sintaxis: ./mi_link disco /ruta_fichero_original /ruta_enlace\n" RESET);
        return FALLO;
    }

    char * ruta_fichero_original = argv[2];
    char * ruta_enlace = argv[3];

   // si las rutas son de directorios--> devolver error
    if (ruta_fichero_original[strlen(ruta_fichero_original) - 1] == '/' || ruta_enlace[strlen(ruta_enlace) - 1] == '/') {
        fprintf(stderr, RED "Las rutas deben corresponder a ficheros!\n" RESET);
        return FALLO;
    }
    // Montar el disco
    if (bmount(argv[1]) == FALLO) {
          fprintf(stderr, RED "mi_link.c: Error al montar el disco\n"RESET);
        return FALLO;
    }
    //enlazar los dos ficheros
    int error = mi_link(argv[2], argv[3]);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
    // desmontar el dispositivo
    if (bumount() == FALLO){
        fprintf(stderr, RED "mi_link.c: Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    return EXITO;
}


