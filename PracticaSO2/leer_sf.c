#include "ficheros_basico.h"


//NIVEL2: 
       
        

int main(int argc, char **argv){

    if (argc!=2){
        fprintf(stderr, RED NEGRITA"Sintaxis incorrecta :./leer_sf <nombre_dispositivo>"RESET);
        return FALLO;
    }
    char nomDispositivo = argv[1];

    //montar y desmontar el dispositivo virtual
    if (bmount(nomDispositivo)<1){
        fprintf(stderr, RED "Se ha producido un error al montar el dispositivo %s.\n"RESET, nomDispositivo);
    return FALLO;  
      }

      struct  superbloque SB;
      struct inodo inodo;
      

    //mostrar los campos del superbloque
    printf("DATOS DEL SUPERBLOQUE\n.");
    printf("posPrimerBloqueMB = %i\n",SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB = %i\n", SB.posUltimoBloqueMB);

    printf("posPrimerBloqueAI = %i\n",SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %i\n", SB.posUltimoBloqueAI);

    printf("posPrimerBloqueDatos = %i\n",SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %i\n", SB.posUltimoBloqueDatos);

    printf("posInodoRaiz = %i\n",SB.posInodoRaiz);
    printf("posPrimerInodoLibre = %i\n",SB.posPrimerInodoLibre);

    printf("cantBloquesLibres = %i\n",SB.cantBloquesLibres);

    printf("cantInodosLibres  = %i\n",SB.cantInodosLibres);
    printf("totBloques  = %i\n",SB.totBloques);
    printf("totInodos  = %i\n",SB.totInodos);
    //mostrar el tamaño de SB y inodo 
    printf("\nsizeof struct superbloque: %i\n", sizeof(SB));
    printf("sizeof struct inodo: %i", sizeof(inodo));

    //recorrido de la lista de inodos libres
    printf("RECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    struct inodo inodos [BLOCKSIZE/INODOSIZE];
   int conInodos = 0;

    
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++){
       
       if (bread(i, inodos)<1){
        fprintf(stderr, RED"Error al montar el dispositivo\n." RESET);
        return FALLO;
       }
       //inodos por bloque
       for (int j = 0; j<BLOCKSIZE/INODOSIZE;j++){
        //si el inodo es libre
        if (inodos[j].tipo=='l'){
            conInodos++;
            print("%d", conInodos);
        //hemos recorrido todos los inodos
        }else if (conInodos==SB.totInodos){
              printf("-1 \n");  
        }
       }

    }


      //Desmontar el dispositivo virtual
 if (bumount(nomDispositivo)<0){

    fprintf(stderr, RED"Error al desmontar el dispositivo %s.\n" RESET, nomDispositivo);
    return FALLO;
  }

  return EXITO;
}