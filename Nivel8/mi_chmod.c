#include "directorios.h"

/**
 * Programa que cambia los permisos de un fichero/directorio, llamando a la función mi_chmod().
 * 
*/

int main (int argc, char ** argv){
    if (argc!=4){
        fprinf(stderr, RED "Sintaxis: ./mi_chmod <disco> <permisos> </ruta>\n"RESET);
        return FALLO;
    }

    //comprobamos que los permisos son correctos

    //los permisos se indican en octal

    int permisos = atoi(argv[2]);
    if (permisos<0 || permisos >7){

        fprintf(stderr, RED "Error: permisos no validos\n"RESET);
        return FALLO;
    }

    if (mi_chmod(argv[1], permisos)<0){
        fprintf(stderr, RED "Error al cambiar los permisos del %s"RESET, argv[1]);
        return FALLO;
    }













    return EXITO;
}