#include "directorios.h"
/**
 * Programa que borra todo el contenido de un directorio no vacío de forma recursiva.
 * 
*/

int main (int argc, char ** argv){

//mi_rmdir --> borrar directorios vacíos
//mi_rrm--> borrar ficheros
if (argc!=3){
    fprintf(stderr,RED "Sintaxis: ./mi_rm_r disco /ruta\n "RESET );
    return FALLO;
}

    mi_rm_r();
return EXITO;
}