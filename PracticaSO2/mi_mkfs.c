#include "bloques.h"
#include <stdio.h>


/**
 * Programa principal
 * Formatea el dispositivo virtual con el tamaño adecuado de bloques, nbloques. 
 * @param   argc numero de parametros en argv
 * @param   argv    cadena de comandos
 * @return 0 si el programa se ejecuto correctamente
*/
 

int main(int argc, char **argv){

char *nombre;
if (argc!=3){
    fprintf(stderr, RED"Sintaxis incorrecta:./mi_mkfs <nombre_dispositivo> <nbloques>."RESET);
    return FALLO;
}
//obtener nombre de dispositivo
char * nombre = argv[1];
//numero de bloques
int nbloques = atoi(argv[2]);

//montar el dispositivo virtual



}
