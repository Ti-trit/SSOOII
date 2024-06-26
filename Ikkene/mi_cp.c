/**
* @author Khaoula Ikkene
**/
#include "directorios.h"
/**
 * Programa que copia un fichero en otro directorio
 * 
 * 
*/

int main (int argc, char ** argv){

if (argc!=4){
    fprintf(stderr,RED "Sintaxis: $ ./mi_cp <disco> </origen/nombre> </destino/>\n"RESET );
    return FALLO;
}

   //montar el disco

if (bmount(argv[1])==FALLO){
    fprintf(stderr, RED "mi_cp.c: Error al montar el disco\n"RESET);
    return FALLO;
    }


char *rutaAntigua = argv[2];
char *destino = argv[3];
int error = mi_cp(rutaAntigua, destino);
    if (error<0){
        return error;
    }

      //desmontar el disco
    if (bumount()==FALLO){
        fprintf(stderr, RED "mi_cp.c: Error al desmontar el"RESET);
        return FALLO;
    }
return EXITO;
}

