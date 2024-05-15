#include "directorios.h"


/**
 * Programa principal
 * Formatea el dispositivo virtual con el tamaño de nbloques. 
 * @param   argc número de parametros en argv
 * @param   argv    cadena de comandos
 * @return  0 si el programa se ejecuto correctamente
*/
 

int main(int argc, char **argv){

if (argc!=3){
    fprintf(stderr, RED"Sintaxis incorrecta:./mi_mkfs <nombre_dispositivo> <nbloques>.\n"RESET);
    return FALLO;
}
//obtener nombre de dispositivo
char * camino = argv[1];
//número de bloques
int nbloques = atoi(argv[2]);

 if(nbloques < 4){
       fprintf(stderr, RED "Error: El número de bloques debe ser mayor o igual a 4\n" RESET);
       return FALLO;
    }

//montar el dispositivo virtual
  if (bmount(camino)<0){
    fprintf(stderr, RED "Se ha producido un error al montar el dispositivo.\n"RESET);
    return FALLO;
  }
 

    //buffer 
    unsigned char buf [BLOCKSIZE];
    //inicializar los elementos del buffer a 0s
    memset(buf, 0, BLOCKSIZE);
    //Escritura en el dispositivo
  for (int i = 0; i<nbloques; i++){
   if ( bwrite(i, buf)<0){
    fprintf(stderr, RED"Error al escribir en el bloque %i\n."RESET, i);
    return FALLO;
   }

  }
  //llamadas a los metodos de inicialización de campos de metadatos
  if (initSB(nbloques, nbloques/4)<0){
    fprintf(stderr, RED"Error initSB()\n"RESET);
    return FALLO;
  }
  if (initMB()<0){
    fprintf(stderr, RED"Error initMB()\n"RESET);
    return FALLO;
  }
  if (initAI()<0){
    fprintf(stderr, RED"Error initAI()\n"RESET);
    return FALLO;
  }

  //Creamos el directorio raiz
  if (reservar_inodo('d', 7) < 0){
    fprintf(stderr, RED"Error al reservar inodo\n"RESET);
    return FALLO;
  }

  //desmontar el dispositivo virtual
  if (bumount(camino)<0){

    fprintf(stderr, RED"Error al desmontar el dispositivo.\n" RESET);
    return FALLO;
  }
  return EXITO;

}