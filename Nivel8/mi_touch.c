#include "directorios.h"

/**
 * Programa que crea un fichero
 * 
*/

int main (int argc, char**argv){

    if (argc!=2){
        fprintf(stderr, RED "Sintaxis: <mi_touch> <fichero.txt>\n"RESET);
        return FALLO;

    }

        int id = open(argv[1], O_CREAT| S_IRUSR | S_IWUSR);
        if(id<0){
            fprintf(stderr, RED "Error al crear el fichero %s\n"RESET, argv[1]);
            return FALLO;
        }


    return EXITO;
}