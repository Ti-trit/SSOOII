#include "directorios.h"

/**
 * Programa que crea un fichero
 * @authors Khaoula Ikkene, Francesc Gayá Piña
 * 
*/

int main (int argc, char**argv){

    if (argc!=4){
        fprintf(stderr, RED "Sintaxis: <mi_touch> <nombre_dispositivo> <permisos> </ruta>\n"RESET);
        return FALLO;

    }

        char *camino = argv[3];
        if (camino[strlen(camino)-1] == '/') {
        fprintf(stderr, RED "mi_touch.c: mi_touch se usa para crear ficheros!.\n" RESET);
        return FALLO;
    }

        int permisos = atoi(argv[2]);
    // Comprobar permisos
    if (permisos <0 || permisos > 7) {
        fprintf(stderr, RED "Error: modo inválido<<%i>>.\n" RESET, permisos);
        return FALLO;
    }

    // montar el dispositivo
    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, RED "mi_touch.c: Error al montar el dispositivo virtual\n" RESET);
        return FALLO;
    }

    // Crear fichero
    int error = mi_creat(camino, permisos);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
      // return error;
    }

    // desmontar el dispositivo disco
    if (bumount() == FALLO) {
        fprintf(stderr, RED "mi_touch.c: Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    return EXITO;
}