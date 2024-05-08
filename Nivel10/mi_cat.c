#include "directorios.h"
#define tambuffer 1500
/**
 * Programa que muestra todo el contenido de uun fichero
*/


int main (int argc, char ** argv){


    //comprobamos la sintaxis
    if (argc!=3){
        fprintf(stderr, RED "Sintaxis: mi_cat <disco> </ruta_fichero>\n"RESET);
        return FALLO;
    }

    //montar el disco

if (bmount(argv[1])==FALLO){
          fprintf(stderr, RED "mi_cat.c: Error al montar el disco\n"RESET);
            return FALLO;
    }

    //comprobamos que la ruta de un fichero
    char * ruta = (char*)argv[2];
    if (ruta[strlen(ruta)-1]=='/'){
        fprintf(stderr, RED "mi_cat.c: La ruta no es correcta\n"RESET);
        return FALLO;
    }
unsigned char buffer[tambuffer];
memset(buffer, 0, tambuffer);
int offset = 0;
int leidos_aux = 0;
int bytes_leidos =0;
     leidos_aux = mi_read(ruta, buffer, offset, tambuffer);
while (leidos_aux>0){//mientras hay contenido en el fichero 
    bytes_leidos+=leidos_aux; 
       //imprimir por pantalla el contenido del fichero
        write(1, buffer, leidos_aux);

       // printf("%s", buffer);
      //actualizamos el offset de lectura

    offset+=leidos_aux;
    memset(buffer, 0, tambuffer);
    leidos_aux=mi_read(ruta, buffer,offset, tambuffer);
}

if (bytes_leidos<0){
     mostrar_error_buscar_entrada(bytes_leidos);
        bytes_leidos = 0;
}

// struct superbloque SB;
// if (bread(posSB, &SB)<0){
//     fprintf(stderr, RED "Error al leer el superbloque\n"RESET);
//     return FALLO;
// }
// //buscamos el inodo del fichero
// unsigned int p_inodo_dir=0, p_inodo=0, p_entrada=0;
// struct inodo inodo;
// int error = buscar_entrada(ruta, p_inodo_dir,p_inodo,p_entrada,0,2);
// if (error<0){
//     mostrar_error_buscar_entrada(error);
//     return FALLO;
// }
// if (leer_inodo(p_inodo,&inodo)==FALLO){
//     fprintf(stderr, RED "mi_cat.c: Error al leer el inodo del fichero\n"RESET);
//     return FALLO;
// }
// if (bytes_leidos!=inodo.tamEnBytesLog){
//     fprintf(stderr, RED "El tamaño de bytes leidos no coincide con el tamaño de bytes del fichero\n"RESET);
// }
fprintf(stdout, "\nTotal_leidos : %i\n", bytes_leidos);

    //desmontar el disco
    if (bumount()==FALLO){
        fprintf(stderr, RED "mi_cat.c: Error al desmontar el disco\n"RESET);
        return FALLO;
    }

return EXITO;
}
