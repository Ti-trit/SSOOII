#include "directorios.h"
/**
 * Programa que borra un fichero , llamando a mi_unlink()
*/

int main (int argc, char ** argv){
    if (argc!=3){
        fprintf(stderr,RED "Sintaxis: ./mi_rm disco /ruta\n "RESET );
        return FALLO;
    }

    //montar el disco

    if (bmount(argv[1])==FALLO){
        fprintf(stderr, RED "mi_rm.c: Error al montar el disco\n"RESET);
        return FALLO;
    }

    char * ruta = argv[2];

    if (ruta[strlen(ruta)-1]=='/'){//es un directorio
        fprintf(stderr, RED "mi_rm.c: Es un directorio\n" RESET);
        return FALLO;
    }

    if (mi_unlink(ruta)<0) {
        //fprintf(stderr, RED "mi_rm.c: Fallo al borrar el fichero\n"RESET);
        return FALLO;
    }

    //desmontar el disco
    if (bumount()==FALLO){
        fprintf(stderr, RED RESET);
        return FALLO;
    }
    return EXITO;
}