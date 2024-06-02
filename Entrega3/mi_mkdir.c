#include "directorios.h"

/**
 * Programa que crea un fichero o directorio
*/
int main (int argc, char *argv[]){
    //comporbar sintaxis
    if (argc!=4){
        fprintf(stderr, RED "Sintaxis: ./mi_mkdir <nombre_dispositivo> <permisos> </ruta_directorio/>\n " RESET);
        return FALLO;
    }
   
   //La ruta es de un directorio?
    char * camino = argv[3];
    if (camino[strlen(camino) - 1] != '/') {
        fprintf(stderr, RED"La ruta no termina en '/'\n"RESET);
        return FALLO;
    }
    
    unsigned char permisos = atoi(argv[2]);
    if (permisos < 0 || permisos > 7) {
        fprintf(stderr, RED "Error: modo inválido<<%i>>\n" RESET, permisos);
        return FALLO;
    }

     //montar el dispositivo
    if (bmount(argv[1])<0){
        fprintf(stderr, RED "mi_mkdir.c: Error al montar el dispositivo\n"RESET);
        return FALLO;
    }

    int error = mi_creat(camino, permisos); 
    if (error< 0) {
       mostrar_error_buscar_entrada(error);
        return FALLO;
    }
     //desmontar el dispositivo
    if (bumount()==FALLO){
        fprintf(stderr, RED "mi_mkdir.c: Error al desmontar el dispositivo\n"RESET);
        return FALLO;
    }

    return EXITO;

}