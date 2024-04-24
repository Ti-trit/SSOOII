#include "directorios.h"

/**
 * Programa que crea un fichero o directorio
*/
int main (int argc, char *argv[]){
    //comporbar sintaxis
    if (argc<3){
        fprintf(stderr, RED "Sintaxis:./mi_mkdir <disco> <permisos> </ruta>.\n " RESET);
        return FALLO;
    }
    //montar el dispositivo
    if (bmount(argv[1]<0)){
        fprintf(stderr, RED "mi_mkdir.c: Error al montar el dispositivo\n"RESET);
        return FALLO;
    }

    //comprobar si termina en '/'
    int len = strlen(argv[3]);
    if (argv[3][len - 1] != '/') {
        fprintf(stderr, RED"La ruta no termina en '/'\n"RESET);
        return FALLO;
    }
    int permisos = atoi(argv[2]);
    if (permisos < 0 || permisos > 7) {
        fprintf(stderr, RED"mi_mkdir.c: los permisos no son válidos\n"RESET);
        return FALLO;
    }

    int (mi_creat(argv[3], argv[2] < 0)) {
        fprintf(stderr, RED"Error al llamar a mi_creat() en mi_mkdir.c\n"RESET);
        return FALLO;
    }
     //desmontar el dispositivo
    if (bumount()==FALLO){
        fprintf(stderr, RED "mi_mkdir.c: Error al desmontar el dispositivo\n"RESET);
        return FALLO;
    }

    return EXITO;

}