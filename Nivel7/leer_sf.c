#include "directorios.h"
        

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
    #if DEBUGN3
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
    #endif
    //reservar inodos
    #if DEBUGN4
    int inodoResv = reservar_inodo('f', 6);
        struct inodo inodo;
     if (leer_inodo(inodoResv,&inodo)<0){
      perror("leer_inodo error");
      return FALLO;
     }
    if (bread(posSB, &SB)<0){
       fprintf(stderr, RED"Error al leer el superbloque\n" RESET);
      return FALLO;
    }
    
    printf("\nINODO 1. TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n");

    if (traducir_bloque_inodo(&inodo, 8, 1)<0){
      perror("Error al traducir el bloque 8");
      return FALLO;
    }
    if (traducir_bloque_inodo(&inodo, 204, 1)<0){
       perror("Error al traducir el bloque 204");
      return FALLO;
    }
    if (traducir_bloque_inodo(&inodo, 30004, 1)==FALLO){
       perror("Error al traducir el bloque 30004");
      return FALLO;
    }
    if ( traducir_bloque_inodo(&inodo, 400004, 1)==FALLO){
       perror("Error al traducir el bloque 400004");
      return FALLO;
    }
    if (traducir_bloque_inodo(&inodo, 468750, 1)==FALLO){
      perror("Error al traducir el bloque 468750");
      return FALLO;
    }
   
    
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    printf("DATOS DEL INODO RESERVADO %d\n", inodoResv);
    printf("tipo: %c\n",inodo.tipo);
    printf("permisos: %i\n", inodo.permisos);

    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
     ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

 // Imprime los tiempos del inodo en la salida estándar.
    printf("atime: %s \nmtime: %s \nctime: %s\n", atime, mtime, ctime);

    printf("nlinks: %i\n", inodo.nlinks);
    printf("tamEnBytesLog: %i\n", inodo.tamEnBytesLog);
    printf("\nnumBloquesOcupados: %i\n", inodo.numBloquesOcupados);

    printf("SB.posPrimerInodoLibre = %i\n", SB.posPrimerInodoLibre);
   
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
     #endif

  char *caminos[] = {"pruebas/","/pruebas/" , "/pruebas/docs/", "/pruebas/", "/pruebas/docs/", "/pruebas/docs/doc1", "/pruebas/docs/doc1/doc11", "/pruebas/", "/pruebas/docs/doc1", "/pruebas/docs/doc1", "/pruebas/casos/", "/pruebas/docs/doc2"};
  for (int i =0; i<sizeof(caminos)/sizeof(char); i++){
    int reservar =1;
    if (i==1 || i == 9){
      reservar = 0;
    }
    mostrar_buscar_entrada(caminos[i], reservar);
  }

     

      //Desmontar el dispositivo virtual
 if (bumount(argv[1])<0){

    fprintf(stderr, RED"Error al desmontar el dispositivo %s.\n" RESET, nomDispositivo);
    return FALLO;
  }

  return EXITO;
}

void mostrar_buscar_entrada(char *camino, char reservar){
  unsigned int p_inodo_dir=0,  p_inodo=0, p_entrada=0;
  int error;
    printf("\ncamino: %s, reservar: %d\n", camino, reservar);
    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6);
    if (error<0){
      mostrar_error_buscar_entrada(error);
      //perror();

    }

  printf("**********************************************************************\n");
  return EXITO;

}

