#include "ficheros_basico.h"

        

int main(int argc, char **argv){

    if (argc!=2){
        fprintf(stderr, RED NEGRITA"Sintaxis incorrecta :./leer_sf <nombre_dispositivo>\n"RESET);
        return FALLO;
    }
    const char *nomDispositivo = argv[1];

    //montar el dispositivo virtual
    if (bmount(nomDispositivo)<0){
        fprintf(stderr, RED "Error al montar el dispositivo %s.\n"RESET, nomDispositivo);
    return FALLO;  
      }

      struct  superbloque SB;
      //Leer el superbloque 
      if (bread(posSB, &SB) <0)
    {
        fprintf(stderr, RED"Error de lectura del superbloque.\n"RESET);
        return FALLO;
    }
      
    //mostrar los campos del superbloque
    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB = %d\n",SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB = %d\n", SB.posUltimoBloqueMB);

    printf("posPrimerBloqueAI = %d\n",SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %d\n", SB.posUltimoBloqueAI);

    printf("posPrimerBloqueDatos = %d\n",SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %d\n", SB.posUltimoBloqueDatos);

    printf("posInodoRaiz = %d\n",SB.posInodoRaiz);
    printf("posPrimerInodoLibre = %d\n",SB.posPrimerInodoLibre);

    printf("cantBloquesLibres = %i\n",SB.cantBloquesLibres);

    printf("cantInodosLibres  = %i\n",SB.cantInodosLibres);
    printf("totBloques  = %d\n",SB.totBloques);
    printf("totInodos  = %d\n",SB.totInodos);
    //mostrar el tamaño de SB y inodo 
    printf("\nsizeof struct superbloque: %ld\n", sizeof(struct superbloque));
    printf("sizeof struct inodo: %ld\n", sizeof(struct inodo));

    //recorrido de la lista de inodos libres
    printf("RESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
    bloque_reservado=reservar_bloque()
    if (bloque_reservado < 0){
      fprintf(stderr, RED"Error al reservar un bloque\n"RESET)
      return FALLO;
    } else {
      printf("Se ha reservado el bloque físico %d que era el 1º libre indicado por el MB",bloque_reservado);
    }
    if (bread(posSB, &SB)<0){
      fprintf(stderr, RED"Error de lectura del superbloque.\n"RESET);
      return FALLO;
    }

    printf("SB.cantBloquesLibres= %i\n"SB.cantBloquesLibres);

    if (liberar_bloque(bloque_reservado)) {
      fprintf(stderr, RED"Error al liberar un bloque\n"RESET)
      return FALLO;
    }
    
    printf("Liberamos ese bloque y después SB.cantBloquesLibres= %i\n"SB.cantBloquesLibres);
    
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
            if (conInodos<28 || (conInodos<SB.totInodos && conInodos>24980)){
            printf("%d ", conInodos);
            } else if (conInodos==29){
              printf("... ");
            }else if (conInodos==SB.totInodos){
              printf("-1 \n");  
        }
       

    }
    }
    }


      //Desmontar el dispositivo virtual
 if (bumount(argv[1])<0){

    fprintf(stderr, RED"Error al desmontar el dispositivo %s.\n" RESET, nomDispositivo);
    return FALLO;
  }

  return EXITO;
}