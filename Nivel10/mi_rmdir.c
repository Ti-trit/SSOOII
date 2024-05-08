#include "directorios.h"
/**
 * Programa que borra un directorio vacío, llamando a mi_unlink()
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

    if (ruta[strlen(ruta)-1]!='/'){//es un fichero
        fprintf(stderr, RED "mi_rm.c: Esta programa no borra ficheros\n" RESET);
        return FALLO;
    }

    struct inodo inodo_dir;
    if (leer_inodo(p_inodo_dir, &inodo_dir)==FALLO){
        fprintf(stderr, RED"mi_rm_r: Error al leer el inodo del directorio\n"RESET);
        return FALLO;
    }
    int nEntradas = inodo_dir.tamEnBytesLog/sizeof(struct entrada);
    if (nEntradas!=0){
        fprintf(stderr, RED "mi_rm_r: l directorio no es vacío\n"RESET);
        return FALLO;
    }

    if (mi_unlink(ruta)==FALLO) {
        fprintf(stderr, RED "mi_rm.c: Fallo al borrar el directorio %s\n"RESET, ruta);
        return FALLO;
    }

    //desmontar el disco
    if (bumount()==FALLO){
        fprintf(stderr, RED RESET);
        return FALLO;
    }
    return EXITO;
}
