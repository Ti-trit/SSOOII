#include "directorios.h"

#define tambuffer 1500
/**
 * Programa que muestra todo el contenido de uun fichero
*/
int main(int argc, char const *argv[]){
    if(argc != 3){
        fprintf(stderr, RED "Sintaxis: ./mi_cat <disco> </ruta_fichero>\n" RESET);
        return FALLO;
    }


     char *ruta = (char*)argv[2];
   int offset=0, bytes_leidos = 0;
    char buffer[tambuffer];
    memset(buffer, 0, tambuffer);
    if ((ruta[strlen(ruta) - 1]) == '/') {
        fprintf(stderr, RED "mi_cat.c: La ruta no es correcta\n"RESET);
        return FALLO;
    }
    if (bmount(argv[1]) == FALLO) {
          fprintf(stderr, RED "mi_cat.c: Error al montar el disco\n"RESET);
        return FALLO;
    }

    int leidos_aux = mi_read(ruta, buffer, offset, tambuffer);
    while (leidos_aux > 0){ 
        bytes_leidos += leidos_aux;
        if(write(1, buffer, leidos_aux) < 0){
          //  mostrar_error_buscar_entrada(bytes_leidos);
            return FALLO;
        }
        memset(buffer, 0, tambuffer);
        offset += tambuffer;
        leidos_aux = mi_read(ruta, buffer, offset, tambuffer);
    }
    if (leidos_aux < 0) {
        bytes_leidos = 0;
    }
    fprintf(stderr, "\nTotal_leidos: %i\n", bytes_leidos);
    if (bumount() == FALLO){
        fprintf(stderr, RED "mi_cat.c: Error al desmontar el disco.\n" RESET);
        return FALLO;
    }
    return EXITO;
}