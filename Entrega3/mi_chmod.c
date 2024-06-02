#include "directorios.h"

/**
 * Programa que cambia los permisos de un fichero/directorio.
 * 
*/

int main (int argc, char ** argv){
    if (argc!=4){
        fprintf(stderr, RED "Sintaxis: ./mi_chmod <nombre_dispositivo> <permisos> </ruta>\n"RESET);
        return FALLO;
    }

    //comprobamos que los permisos son correctos
    unsigned char permisos = atoi(argv[2]);
    if (permisos<0 || permisos >7){

        fprintf(stderr, RED "Error: permisos no validos\n"RESET);
        return FALLO;
    }

 // Montar el dispositivo virtual
    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, RED "mi_chmod.c: Error al montar el dispositivo\n" RESET);
        return FALLO;
    }
    int error = mi_chmod(argv[3], permisos);
    if (error<0){
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

if (bumount() == FALLO){
        fprintf(stderr, RED "mi_chmod.c: Error al desmontar el dispositivo\n" RESET);
        return FALLO;
    }



    return EXITO;
}