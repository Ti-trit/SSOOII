#include "directorios.h"


int main(int argc, char *argv[]){


    if (argc!=4){
        fprintf(stderr, RED "Sintaxis: permitir <nombre_dispositivo> <ninodo> <permisos>\n"RESET);
        return FALLO;
    }
    // const char dispositivo = argv[1];
    int ninodo = atoi(argv[2]);
     unsigned char permisos = atoi(argv[3]);

    //montar el dispositivo

    if (bmount(argv[1])==FALLO){
          fprintf(stderr, RED "Se ha producido un error al montar el dispositivo.\n"RESET);
            return FALLO;
    }

    //llamamos a mi_chmod_f
    if (mi_chmod_f(ninodo, permisos)==FALLO){
        perror("mi_chmod_f");
        return FALLO;
    }

    //desmontar el dispositivo

    if (bumount(argv[1])==FALLO){
         fprintf(stderr, RED"Error al desmontar el dispositivo.\n" RESET);
    return FALLO;
    }
    return EXITO;

}