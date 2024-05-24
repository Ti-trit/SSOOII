#include "directorios.h"
/**
 * Programa que borra todo el contenido de un directorio no vacío de forma recursiva.
 * 
*/

int main (int argc, char ** argv){

if (argc!=3){
    fprintf(stderr,RED "Sintaxis: ./mi_rm_r disco /ruta \n "RESET );
    return FALLO;
}

   //montar el disco

if (bmount(argv[1])==FALLO){
          fprintf(stderr, RED "mi_rm_r.c: Error al montar el disco\n"RESET);
            return FALLO;
    }

char * ruta = argv[2];
if (ruta[strlen(ruta)-1]!='/'){//es un fichero
        fprintf(stderr, RED "mi_rm_r: La ruta no es de un directorio %s\n"RESET, ruta);
        return FALLO;
    
}
    if (mi_rm_r(argv[2])<0){
     //   perror("Error");
        return FALLO;
    }

      //desmontar el disco
    if (bumount()==FALLO){
        fprintf(stderr, RED "mi_rm_r: Error al desmontar el"RESET);
        return FALLO;
    }
return EXITO;
}