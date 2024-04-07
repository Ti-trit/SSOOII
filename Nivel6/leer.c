#include "ficheros.h"

#define tambuffer 1500 //Tamaño del buffer de lectura

int main(int argc, char const *argv[]) {

    if (argc!=3){
        fprintf(stderr, RED "Sintaxis: permitir <nombre_dispositivo> <ninodo>\n"RESET);
        return FALLO;
    }

    //const char dispositivo = argv[1];
    int ninodo = atoi(argv[2]);
    
    //montar el dispositivo

    if (bmount(argv[1])==FALLO){
          fprintf(stderr, RED "Se ha producido un error al montar el dispositivo(leer.c).\n"RESET);
            return FALLO;
    }

    struct superbloque SB;
    //Leer superbloque
    if (bread(0, &SB) == FALLO)
    {
        fprintf(stderr, RED"leer.c: Error de lectura del superbloque.\n"RESET);
        return FALLO;
    }


    int offset = 0;
    char buffer_texto[tambuffer];
    int numBytesLeidos = 0;
    int leidos = 0;
    memset(buffer_texto,0,tambuffer);
        //obtener los bytes leIdos
    leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
    /*if ( leidos < 0){
        fprintf(stderr, RED"Se ha producido un error al leer el bloque\n"RESET);
        return FALLO;
    }*/
    
    while (leidos > 0) {
        numBytesLeidos += leidos;
        write(1, buffer_texto, leidos);
        memset(buffer_texto,0,tambuffer);
        offset += tambuffer;
        leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer) ;

        /*if (leidos < 0){
            fprintf(stderr, RED"Se ha producido un error al leer el bloque\n"RESET);
            return FALLO;
        }*/
    }

   struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED"Error al leer el inodo (leer.c).\n"RESET);
        return FALLO;
    }

        char string[128];
        sprintf(string, "\ntotal_leidos: %d\ntamEnBytesLog: %d\n", numBytesLeidos, inodo.tamEnBytesLog);
        write(2, string, strlen(string));
       // fprintf(stderr, "total_leidos: %d\ntamEnBytesLog: %d\n", numBytesLeidos, inodo.tamEnBytesLog);

  
    if (bumount() == FALLO) {
        fprintf(stderr, RED"Error al desmontar el dispositivo.\n"RESET);
        return FALLO;
    }
return EXITO;
}
