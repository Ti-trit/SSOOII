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
    #if DEBUGN2
    printf("\nsizeof struct superbloque: %ld\n", sizeof(struct superbloque));
    printf("sizeof struct inodo: %ld\n", sizeof(struct inodo));
    #endif
    printf("\n\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
    int bloque_reservado=reservar_bloque();
    if (bloque_reservado < 0){
      fprintf(stderr, RED"Error al reservar un bloque\n"RESET);
      return FALLO;
    } else {
      printf("Se ha reservado el bloque físico %d que era el 1º libre indicado por el MB\n ",bloque_reservado);
    }
    if (bread(posSB, &SB)<0){
      fprintf(stderr, RED"Error de lectura del superbloque.\n"RESET);
      return FALLO;
    }

    printf("SB.cantBloquesLibres= %i\n",SB.cantBloquesLibres);

    if (liberar_bloque(bloque_reservado)<0) {
      fprintf(stderr, RED"Error al liberar un bloque\n"RESET);
      return FALLO;
    }
    if (bread(posSB, &SB)<0){
      fprintf(stderr, RED"Error al leer el superbloque\n" RESET);
      return FALLO;
    }
    printf("Liberamos ese bloque y después SB.cantBloquesLibres= %i\n",SB.cantBloquesLibres);


    printf("\n\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
    int bit_leido=leer_bit(posSB);
    if (bit_leido < 0) {
      fprintf(stderr, RED"Error al leer el bit del SB\n" RESET);
    }
    printf("posSB: %i -> leerbit(%i) = %i\n", posSB, posSB, bit_leido);

    bit_leido=leer_bit(SB.posPrimerBloqueMB);
    if (bit_leido < 0) {
      fprintf(stderr, RED"Error al leer el primer bit del bloque de MB\n" RESET);
    }
    printf("SB.posPrimerBloqueMB: %i -> leer_bit(%i) = %i\n", SB.posPrimerBloqueMB, SB.posPrimerBloqueMB, bit_leido);

    bit_leido=leer_bit(SB.posUltimoBloqueMB);
    if (bit_leido < 0) {
      fprintf(stderr, RED"Error al leer el bit del ultimo bloque de MB\n" RESET);
    }
    printf("SB.posUltimoBloqueMB: %i -> leer_bit(%i) = %i\n", SB.posUltimoBloqueMB, SB.posUltimoBloqueMB, bit_leido);
    
    bit_leido=leer_bit(SB.posPrimerBloqueAI);
    if (bit_leido < 0) {
      fprintf(stderr, RED"Error al leer el primer bit del bloque AI\n" RESET);
    }
    printf("SB.posPrimerBloqueAI: %i -> leer_bit(%i) = %i\n", SB.posPrimerBloqueAI, SB.posPrimerBloqueAI, bit_leido);

    bit_leido=leer_bit(SB.posUltimoBloqueAI);
    if (bit_leido < 0) {
      fprintf(stderr, RED"Error al leer el ultimo bit del bloque AI\n" RESET);
    }
    printf("SB.posUltimoBloqueAI: %i -> leer_bit(%i) = %i\n", SB.posUltimoBloqueAI, SB.posUltimoBloqueAI, bit_leido);

    bit_leido=leer_bit(SB.posPrimerBloqueDatos);
    if (bit_leido < 0) {
      fprintf(stderr, RED"Error al leer el primer bloque de datos\n" RESET);
    }
    printf("SB.posPrimerBloqueDatos: %i -> leer_bit(%i) = %i\n", SB.posPrimerBloqueDatos, SB.posPrimerBloqueDatos, bit_leido);

    bit_leido=leer_bit(SB.posUltimoBloqueDatos);
    if (bit_leido < 0) {
      fprintf(stderr, RED"Error al leer el ultimo bloque de datos\n" RESET);
    }
    printf("SB.posUltimoBloqueDatos: %i -> leer_bit(%i) = %i\n", SB.posUltimoBloqueDatos, SB.posUltimoBloqueDatos, bit_leido);

    struct inodo inodo_raiz [BLOCKSIZE/INODOSIZE];
    int ninodo = 0;
    if (leer_inodo(ninodo, inodo_raiz) < 0) {
      fprintf(stderr, RED"Error al leer el inodo raiz\n" RESET);
    }
    printf("\nDATOS DEL DIRECTORIO RAIZ\n");
    printf("tipo: %c\n", inodo_raiz->tipo);
    printf("permisos: %i\n", inodo_raiz->permisos);

    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];

    // Leer los datos del inodo especificado por 'ninodo'.

    // Convierte el tiempo de último acceso del inodo a formato local y luego a cadena.
    ts = localtime(&inodo_raiz->atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);

    // Convierte el tiempo de última modificación del inodo a formato local y luego a cadena.
    ts = localtime(&inodo_raiz->mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);

    // Convierte el tiempo de cambio del estado del inodo a formato local y luego a cadena.
    ts = localtime(&inodo_raiz->ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    // Imprime los tiempos del inodo en la salida estándar.
    printf("atime: %s \nmtime: %s \nctime: %s\n", atime, mtime, ctime);

    printf("nlinks: %i\n", inodo_raiz->nlinks);
    printf("tamEnBytesLog: %i\n", inodo_raiz->tamEnBytesLog);
    printf("numBloquesOcupados: %i\n", inodo_raiz->numBloquesOcupados);
    //------------------------------------------------------------------------------------
    struct inodo inodos [BLOCKSIZE/INODOSIZE];
    int conInodos = 0;

        //recorrido de la lista de inodos libres

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
            #if DEBUGN2
            if (conInodos<28 || (conInodos<SB.totInodos && conInodos>24980)){
            printf("%d ", conInodos);
            } else if (conInodos==29){
              printf("... ");
            }else if (conInodos==SB.totInodos){
              printf("-1 \n");  
        }
        #endif
       

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
