/**
* @authors Khaoula Ikkene, Francesc Gayá Piña
**/
#include "directorios.h"
/**
 * Programa que borra un directorio vacío, llamando a mi_unlink()
*/

int main (int argc, char ** argv){
    if (argc!=3){
        fprintf(stderr,RED "Sintaxis: ./mi_rm disco /ruta\n "RESET );
        return FALLO;
    }

  

    char * ruta = argv[2];

    if (ruta[strlen(ruta)-1]!='/'){//es un fichero
        fprintf(stderr, RED "mi_rm.c: Este programa no borra ficheros\n" RESET);
        return FALLO;
    }

  //montar el disco

    if (bmount(argv[1])==FALLO){
        fprintf(stderr, RED "mi_rm.c: Error al montar el disco\n"RESET);
        return FALLO;
    }


    int error = mi_unlink(ruta);
    if (error<0) {
        mostrar_error_buscar_entrada(error);

       // fprintf(stderr, RED "mi_rm.c: Fallo al borrar el directorio %s\n"RESET, ruta);
        return FALLO;
    }

    //desmontar el disco
    if (bumount()==FALLO){
        fprintf(stderr, RED "mi_rmdir.c: Error al desmotar el disco\n"RESET);
        return FALLO;
    }
    return EXITO;
}