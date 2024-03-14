#include "ficheros_basico.h"

#define byte 8
#define posLibre   0



/**
 * Calcula el tamaño en bloques para el mapa de bits.
 * @param   nbloques
 * @return  tamaño de bloques para el MB
*/

int tamMB(unsigned int nbloques){

        int tambloquesMb= (nbloques/byte)/BLOCKSIZE;
        if ((nbloques/byte) % BLOCKSIZE!=0){
            //nbloques no es multiple de BLOCKSIZE
             tambloquesMb++;
        
        }
      return tambloquesMb;


}

/**
 * Calcula el tamaño en bloques para el array de inodos.
 * @param   ninodos
 * @return  tamaño de bloques para el AI.
*/

int tamAI(unsigned int ninodos){
    int tamAI= (ninodos*INODOSIZE)/BLOCKSIZE;
    if((ninodos*INODOSIZE)%BLOCKSIZE==0){
        // Los inodos caben justo en los bloques.
        return tamAI;
    }else{
        // Añadimos un bloque extra para los inodos restantes.
        return tamAI + 1;
    }
}

/**
 * Inicializa los datos del superbloque.
 * @param   nbloques   número de bloques    
 * @param   ninodos    número de inodos del sistema d ficheros
 * @return  0 o -1 si o no la grabación de estructura fue exitosa
*/
//Representación de la estructura del disco virtual

int initSB(unsigned int nbloques, unsigned int ninodos){

        struct superbloque SB;
        //Datos del mapa de bits
        SB.posPrimerBloqueMB= posSB+tamSB;
        SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;

        //Datos del array de inodos
        SB.posPrimerBloqueAI= SB.posUltimoBloqueMB+1;
        SB.posUltimoBloqueAI=SB.posPrimerBloqueAI+tamAI(ninodos)-1;

        //Datos del bloque de datos
        SB.posPrimerBloqueDatos= SB.posUltimoBloqueAI+1;
        SB.posUltimoBloqueDatos=nbloques-1;
        
        //Datos de los inodos y bloques
        SB.posPrimerInodoLibre=posLibre;
        //Posición del primer inodo libre
        SB.posInodoRaiz=posLibre;
        //Cantidad de bloques libres en el DISCO VIRTUAL
        SB.cantBloquesLibres= nbloques;
        //Cantidad de inodos libres en AI
        SB.cantInodosLibres=ninodos;
        //Cantidad total de bloques
        SB.totBloques=nbloques;
        //Cantidad total de inodos (BLOCKSIZE/4)
        SB.totInodos=ninodos;

       //Escribir la estructura en el bloques posSB 
       if (bwrite(posSB, &SB)<0){
            fprintf(stderr, RED"Error al escribir la estrcutra del bloque.\n"RESET);
            return FALLO;
      }
    return EXITO;


}
/**
 * Inicializa lel mapa de bits poniendo a 1 los bits de metadatos.
 * @return
*/
int initMB(){
    struct superbloque SB;
    unsigned char  bufferMB [BLOCKSIZE];
    memset(bufferMB, 0,sizeof(bufferMB));

    //leer el superbloque para obtener nº de bloques y inodos
    if (bread(posSB, &SB)<0){
        return FALLO;
    }

    int bloquesMetadatos = tamSB + tamMB(SB.totBloques)+ tamAI(SB.totInodos);
       for (int i = 0; i<bloquesMetadatos/byte; i++){
            bufferMB[i]=255; //rellenar con 1
       }

            //no caben en un bloque rellenamos el ultimo bloque manualmente
      if (bloquesMetadatos%byte!=0){
           // bufferMB[(bloquesMetadatos/8)]=256-(int)pow(2.0,(double)(bloquesMetadatos%8));
                bufferMB[bloquesMetadatos/byte]=(256 - (1 << (bloquesMetadatos%byte)));
      }

    
    for(int i = SB.posPrimerBloqueMB; i<SB.posPrimerBloqueMB; i++){
       if (bwrite(i, bufferMB)<0){
        fprintf(stderr, "Error al rellenar el mapa de bits\n"RESET);
        return FALLO;
       }
    }
        //escribir el MB actualizado
   /* if (bwrite(SB.posPrimerBloqueMB, bufferMB)<0){
        fprintf(stderr, RED"Error al escribir el bloque %d \n"RESET, SB.posPrimerBloqueMB);
        return FALLO;
    }*/

    if (bwrite(SB.posPrimerBloqueMB, bufferMB) == FALLO)
    {
        perror("Error initMB bwrite (MB)");
        return FALLO;
    }

 //restar estos bloques de la cantidad de bloques libres
   SB.cantBloquesLibres = SB.cantBloquesLibres-bloquesMetadatos;

    //guardar los cambios en el superbloque
    if (bwrite(posSB, &SB)<0){
        fprintf(stderr, RED"Error al guardar los cambios en el SB  \n"RESET);
        return FALLO;
    }
   
   return EXITO;
}




/**
 * Inicializa la lista de nodos libres
 * @return   0 o -1 si la inicializacion fue exitosa o no
*/

int initAI(){

    struct superbloque SB;
    //Preparar buffer de inodos segun BLOCKSIZE /INODOSIZE
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

        //leer el superbloque para obtener la localizacion del AI
        if ( bread(posSB, &SB) <0){
            return FALLO;
        }

    int contInodos = SB.posPrimerInodoLibre + 1;

    //Leemos cada bloque
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++){
        if (bread(i, inodos) == -1){
            return FALLO;
        }

        //Inicializamos cada inodo del bloque leído
        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++){
            inodos[j].tipo = 'l';
            //Enlazamos con el siguiente inodo libre
            if (contInodos < SB.totInodos){
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos++;
            } else {
                inodos[j].punterosDirectos[0] = UINT_MAX;
                break;
            }
        }
        if (bwrite(i,inodos) == -1){
            return FALLO;
        }

    }
    return EXITO;
}


/**
 * Escribe un 0(libre) o 1(ocupado) en el bloque físico
 * que se pasa por parámetro.
 * @param   nbloque bit del MB a modificar.
 * @param   bit valor a escribir.
 * @return  0 o -1 si la inicializacion fue exitosa o no.
*/

int escribir_bit(unsigned int nbloque, unsigned int bit){
    struct superbloque SB;
    int posbyte = nbloque/8;
    int posbit = nbloque%8;
    
    //leemos el superbloque para obtenir la posición del MB
    if (bread(nbloque, &SB)==FALLO){
        fprintf(stderr, RED "Error al leer el bloque\n"RESET);
        return FALLO;
    }
    //posicion del bloque que contiene el byte
    int nbloqueMB = posbyte/BLOCKSIZE;
    //posición absoluta del bloque que buscamos
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];

    if (bread(nbloqueabs, bufferMB)==FALLO){
        fprintf(stderr, RED "Error al leer el bloque\n"RESET);
        return FALLO;
    }
    //obtener la posición de este byte dentro del bufferMB
    posbyte = posbyte % BLOCKSIZE;

    unsigned char mascara = 128; //10000000
    mascara >>= posbit; //desplazamiento de bits a la derecha

    if(bit == 1){
        bufferMB[posbyte]|= mascara; // Poner el byte a 1.
    }else if(bit == 0){
        bufferMB[posbyte]&= ~mascara; // Poner el byte a 0.
    }else{
        fprintf(stderr, RED "Error, el bit a escribir debe ser 0 o 1\n"RESET);
        return FALLO;
    }

    if (bwrite(nbloqueabs, &SB)<0){
        fprintf(stderr, RED"Error al guardar los cambios en el SB  \n"RESET);
        return FALLO;
    }
    return EXITO;
}

/**
 * Escribe un 0(libre) o 1(ocupado) en el bloque 
 * que se pasa por parámetro.
 * @param   nbloque bit del MB a leer.
 * @return  Devuelve el valor del nbloque en el MB.
*/

char leer_bit(unsigned int nbloque){
    struct superbloque SB;
    int posbyte = nbloque/8;
    int posbit = nbloque%8;
    
    //leemos el superbloque para obtenir la posición del MB
    if (bread(nbloque, &SB)==FALLO){
        fprintf(stderr, RED "Error al leer el bloque\n"RESET);
        return FALLO;
    }
    //posicion del bloque que contiene el byte
    int nbloqueMB = posbyte/BLOCKSIZE;
    //posición absoluta del bloque que buscamos
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];

    if (bread(nbloqueabs, bufferMB)==FALLO){
        fprintf(stderr, RED "Error al leer el bloque\n"RESET);
        return FALLO;
    }
    //obtener la posición de este byte dentro del bufferMB
    posbyte = posbyte % BLOCKSIZE;

    unsigned char mascara = 128; // 10000000
    mascara>>= posbit; // Desplazamiento del bit "posbit" veces
                       // a la derecha.
    mascara&=bufferMB[posbyte]; // operador AND
    mascara>>=(7-posbit); //desplazamos el bit al extremo derecho.

    return mascara;
}

/**
 * Reserva el primer bloque libre que encuentra
 * @return nº del bloque reservado o FALLO si no se pudo reservar un bloque
*/
int reservar_bloque(){

    struct superbloque SB;
    if (bread(posSB, &SB)==FALLO){
            fprintf(stderr, RED "Error al leer el superbloque\n"RESET);
            return FALLO;
    }
        //Hay bloques libres?
    if (SB.cantBloquesLibres>0){ //SÍ
        //Encontrar el primer bloque libre

        //buffer auxiliar
        unsigned char bufferAux [BLOCKSIZE];
        //poner los bits de bufferAux a 1
        memset(bufferAux, 255, BLOCKSIZE);

        unsigned char bufferMB[BLOCKSIZE];
        int nbloqueMB = SB.posPrimerBloqueMB;
        while (nbloqueMB<=SB.posUltimoBloqueMB){
        if (bread(nbloqueMB , bufferMB)<0){
            fprintf(stderr, "Error al leer el bloque\n. "RESET);
            return FALLO;
        }
       if (memcmp(bufferMB, bufferAux, BLOCKSIZE)!=0){ //bloque libre
            break;

       }
        }
        //ahora buefferMB contiene el byte con algún bit a 0
        //localimos de cúal byte se trata
        int posbyte=0;
        for (int i = posbyte; i<BLOCKSIZE; i++){
            if (bufferMB[i]!=255){
                posbyte=i;
                break;
            }
        }
            //localisamos cual bit está a 0 del byte (posbyte)
            unsigned char mascara = 128; //10000000
            int posbit = 0;
            while (bufferMB[posbyte]&mascara){//operador AND para bits
                bufferMB[posbyte]<<=1;  //desplazamiento hacia la izequierda de bits
                posbit++;
            }

        //nº de bloque físico a reservar
        int nbloque = (nbloqueMB*BLOCKSIZE*posbyte)*byte + posbit;
        //escrbimos el bit a 1 para indicar que está reservado
        if (escribir_bit(nbloque,1 )<0){
            fprintf(stderr, RED"Error al escribir el bit 1 para reservar el bloque\n"RESET);
            return FALLO;
        }
        //decrementamos la cantidad de bloques libres
        SB.cantBloquesLibres-=1;
        //limpiamos ese bloque en la zona de datos
        memset(bufferAux, 0, BLOCKSIZE);
        if (bwrite(nbloque, bufferAux)<0){
            fprintf(stderr, RED"Error al limpiar el bloque reservado en al zona de datos\n"RESET);
            return FALLO;
        }
            
             if (bwrite(posSB, &SB)<0){
        fprintf(stderr, RED"Error al guardar los cambios en el SB  \n"RESET);
        return FALLO;
    }
        //devolver el nª del bloque reservado
        return nbloque;


    }else{//No hay bloques libres
        fprintf(stderr, RED"No hay bloques libres.!\n"RESET);
        return FALLO;
    }
}

/**
 * Libera un bloque determinado con la ayuda de escribir_bit()
 * @param nbloque   bloque a liberar
 * @return nbloque o FALLO si no ha ido bien
*/

int liberar_bloque(unsigned int nbloque){
    struct superbloque SB;
    if (bread(posLibre, &SB)==FALLO){
        fprintf(stderr, RED "Error al leer el superbloque\n"RESET);
        return FALLO;
    }
    //Escribimos el bit a 0 para indicar que esta libre
    if (escribir_bit(nbloque, 0)<0){
        fprintf(stderr, RED "Error al escribir 0 para liberar el bloque\n"RESET);
        return FALLO;
    }
//guardar el SB modificado
         if (bwrite(posSB, &SB)<0){
        fprintf(stderr, RED"Error al guardar los cambios en el SB  \n"RESET);
        return FALLO;
    }
    SB.cantBloquesLibres += 1;
    return nbloque;
}

/**
 * Escribir ninodo del AI para volcarlo en inodo 
 * @param ninodo    posición del inodo en el array de inodos
 * @param inodo     el inodo a leer del array de inodos
 * @return  EXITO si todo ha ido bien. FALLO en caso contrario
 * 
*/
int escribir_inodo(unsigned int ninodo,struct inodo *inodo){
    struct superbloque SB;
    //leer el superbloque
    if (bread(posLibre, &SB)==FALLO){
       fprintf(stderr, RED "Error al leer el superbloque\n"RESET);
            return FALLO;
    }

    //obtener el nº de bloque de AI que contiene ninodo
    int nInodosPorBloque = BLOCKSIZE/INODOSIZE;

    int nbloqueAI = ninodo/nInodosPorBloque;
    int nbloqueabs = nbloqueAI+SB.posPrimerBloqueAI;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    if(bread(nbloqueabs,inodos)<0){
        fprintf(stderr, RED "Error al leer el inodo\n"RESET);
            return FALLO;
    }
    //posición absoluta del inodo

    int posInodo = ninodo %(nInodosPorBloque);
    inodo[posInodo]=*inodo;
    if(bwrite(nbloqueabs,inodos)<0){
        fprintf(stderr, RED"Error al guardar los cambios en el SB  \n"RESET);
        return FALLO;
    }
    return EXITO;
}

/**
 * Lee ninodo del AI para volcarlo en inodo 
 * @param ninodo    posición del inodo en el array de inodos
 * @param inodo     el inodo a leer del array de inodos
 * @return  EXITO si todo ha ido bien. FALLO en caso contrario
 * 
*/

int leer_inodo(unsigned int ninodo, struct inodo *inodo){
    //leemos el SB para localizar el AI
    struct superbloque SB;
    if (bread(posLibre, &SB)==FALLO){
       fprintf(stderr, RED "Error al leer el superbloque\n"RESET);
            return FALLO;
    }
    //obtener el nº de bloque de AI que contiene ninodo
    int nInodosPorBloque = BLOCKSIZE/INODOSIZE;
    int nbloqueAI = ninodo/nInodosPorBloque;
    //posición absoluta del inodo
    int nbloqueabs = nbloqueAI+SB.posPrimerBloqueAI;
    
    struct inodo inodos [nInodosPorBloque];
    if (bread(nbloqueabs, inodos)<0){
        fprintf(stderr, RED"Error al leer el inodo\n" RESET);
        return FALLO;
    }
    //guardar el contenido en el inodo
    *inodo = inodos[ninodo%nInodosPorBloque];




    return EXITO;
}
/**
 * Reserva el primer inodo libre que encuentra
 * 
 * @param tipo  tipo de inodo
 * @param permisos  los permisos a asignar al inodo que reservaremos
 * @return posInodoReservado o FALLO si la reserva del inodo ha fallado.
*/

int reservar_inodo(unsigned char tipo, unsigned char permisos){

int posInodoReservado = -1;

struct superbloque SB; 
//leemos el superbloque
if (bread(posLibre, &SB)==FALLO){
  fprintf(stderr, RED "Error al leer el superbloque\n"RESET);
            return FALLO;
    }

    if (SB.cantInodosLibres==0){
          fprintf(stderr, RED "No hay inodos libres para reservar\n"RESET);
            return FALLO;
    }else{
        posInodoReservado=SB.posPrimerInodoLibre;
        //actualizar la lista de inodos libres
        //dado que los inodos libres son en posiciones contiguas el siguiente inodo
        //libre está en SB.posPrimerInodoLibre+1
        SB.posPrimerInodoLibre++;
        //inicializamos los campos de inodo reservado
        struct inodo inodoReservado;
        inodoReservado.tipo=tipo;
        inodoReservado.permisos=permisos;
        inodoReservado.nlinks=1;
        inodoReservado.tamEnBytesLog=0;
        inodoReservado.numBloquesOcupados=0;
        inodoReservado.ctime=time(NULL);
        inodoReservado.atime=time(NULL);
        inodoReservado.mtime=time(NULL);
                for (int i = 0; i<12; i++){
                    if (i<3){
                        inodoReservado.punterosDirectos[i]=0;
                    }
                    inodoReservado.punterosIndirectos[i]=0;
                }
   


        //escribir el inodo
        if (escribir_inodo(posInodoReservado, &inodoReservado)<0){
            fprintf(stderr, RESET"Error al escribir el inodo\n"RESET);
            return FALLO;
        }
        //decrementar la cantidad de inodos libres
        SB.cantInodosLibres--;
        //reescribir el SB para guardar los cambios realizados
        if (bwrite(posSB, &SB)<0){
        fprintf(stderr, RED"Error al guardar los cambios en el SB  \n"RESET);
        return FALLO;
    }

        return posInodoReservado;
    }
}

/**
 * Calculo el rango del bloque lógico
 * @param inodo estructura del inodo
 * @param nblogico bloque logico
 * @param ptr direccion del puntero
*/

int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr) {
    if (nblogico < DIRECTOS) {
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    } else if (nblogico < INDIRECTOS0) {
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    } else if (nblogico < INDIRECTOS1) {
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    } else if (nblogico < INDIRECTOS2) {
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    } else {
        *ptr = 0;
        fprintf(stderr, RED"Error al obtener el rango del bloque lógico\n",RESET);
        return FALLO;
    }

}


/**
 * Calcula el índice del bloque de punteros.
 * 
 * @param nblogico, bloque lógico.
 * @param nivel_punteros, nivel del que cuelgan el bloque de punteros.
 * @return el índice del bloque de punteros o FALLO si los parámetros son incorrectos.
*/

int obtener_indice(unsigned int nblogico, int nivel_punteros){

    if(nblogico<DIRECTOS){ //Dentro de los bloques directos.
        return nblogico;
    }else if((DIRECTOS<=nblogico)&&(nblogico<INDIRECTOS0)){ //Dentro de los bloques indirectos 0.
        return nblogico-DIRECTOS;
    }else if((INDIRECTOS0<=nblogico)&&(nblogico<INDIRECTOS1)){ //Dentro de los bloques indirectos 1.
        if(nivel_punteros=2){
            return (nblogico-INDIRECTOS0)/NPUNTEROS;
        }else if(nivel_punteros=1){
            return (nblogico-INDIRECTOS0)%NPUNTEROS;
        }else{
            fprintf(stderr, RED"Error, el nivel del puntero no es correcto.  \n"RESET);
            return FALLO;
        }
    }else if((INDIRECTOS1<=nblogico)&&(nblogico<INDIRECTOS2)){ //Dentro de los bloques indirectos 2.
            if(nivel_punteros=3){
                return (nblogico-INDIRECTOS1)/(NPUNTEROS*NPUNTEROS);
            }else if(nivel_punteros=2){
                return ((nblogico-INDIRECTOS1)%(NPUNTEROS*NPUNTEROS))/NPUNTEROS;
            }else if(nivel_punteros=1){
                return ((nblogico-INDIRECTOS1)%(NPUNTEROS*NPUNTEROS))%NPUNTEROS;
            }else{
                fprintf(stderr, RED"Error, el nivel del puntero no es correcto.  \n"RESET);
                return FALLO;
            }
    }else{
        fprintf(stderr, RED"Error, El puntero no está dentro del rango máximo.  \n"RESET);
        return FALLO;
    }
}



int traducir_bloque_inodo(struct inodo *inodo, unsigned int nblogico, unsigned char reservar){
    unsigned int ptr = 0;
    unsigned int ptr_ant= 0;
    int nRangoBL = obtener_RangoBL(inodo, nblogico, &ptr); //0:D, 1:I0, 2:I1, 3:I2
    if (nRangoBL<0){
        fprintf(stderr, RED "Error obtener_RangoBL\n" RESET);
        return FALLO;
    }
    
    int nivel_punteros = nRangoBL; //nivel mas alto es el que cuelga directamente del inodo
    unsigned char buffer[BLOCKSIZE];
    int indice = 0;
    while (nivel_punteros>0){ //iteramos por todos los niveles de punteros
        if (ptr==0){    //no cuelgan punteros de bloques
            if (reservar == 0){
                //no notificamos el error
                //bloque inexistente
                return FALLO;
            }else{
                //reservar bloques de punteros y crear punteros del inodo a los bloques de datos
                ptr = reservar_bloque();
                if (ptr<0){
                    fprintf(stderr, RED"Error de reservar_bloque() en traduicr_bloque_inodo\n"RESET);
                    return FALLO;
                }
                inodo->numBloquesOcupados++;
                inodo->ctime = time(NULL); //fecha actual
                if (nivel_punteros==nRangoBL){//NIVEL 0
                    inodo->punterosDirectos[nRangoBL-1]=ptr;
                printf(GRAY "[traducir_bloque_inodo()→ inodo.punterosIndirectos[%i] = %i (reservado BF %i para punteros_nivel%i)]\n"RESET, nRangoBL-1,ptr, nRangoBL-1, nivel_punteros);
                 
            
                }else{
                    //NIVEL1--> el bloque cuelga de otro bloque de punteros
                    buffer[indice]=ptr;
            
                printf(GRAY "[traducir_bloque_inodo()→ punteros_nivel%i [%i] = %i (reservado BF %i para BL %i)]\n" RESET, nivel_punteros, indice, ptr, ptr, nivel_punteros);

                    if (bwrite(ptr_ant, buffer)<0){
                        fprintf(stderr, RED "Error al escribir en el buffer\n" RESET);
                        return FALLO;
                    }
                
                }
            }
            memset(buffer, 0, BLOCKSIZE);
        }else{
            if (bread(ptr, buffer)<0){
                 fprintf(stderr, RED " Error al leer del buffer\n" RESET);
                 return FALLO;   
            }

        }
    indice = obtener_indice(nblogico, nivel_punteros);
    if (indice<0){
        fprintf(stderr, RED "Indice invalido\n"RESET);
        return FALLO;
    }
    ptr_ant = ptr;  //actualizar el puntero
    ptr = buffer[indice]; //desplazar el puntero al siguiente nivel
    nivel_punteros--;
    }

    if (ptr == 0){
        if (reservar==0){
            //error de lectura, bloque inexistente
            return FALLO;
        }else{
            ptr = reservar_bloque();
             if (ptr<0){
                    fprintf(stderr, RED"Error de reservar_bloque() en traduicr_bloque_inodo\n"RESET);
                    return FALLO;
                }
            inodo->numBloquesOcupados++;
            inodo->ctime= time (NULL);
            if (nRangoBL==0){//puntero directo
                inodo->punterosDirectos[nblogico]=ptr; //asignamos la direción del bl. de datos en el inodo
             printf(GRAY "[traducir_bloque_inodo()→ inodo.punterosDirectos[%i] = %i (reservado BF %i para BL %i)]\n"RESET, nRangoBL-1,ptr, reservar, nblogico);


            }else{
                buffer[indice]=ptr;  //asignamos la dirección del bloque de datos en el buffer
                printf(GRAY "[traducir_bloque_inodo()→ punteros_nivel%i [%i] = %i (reservado BF %i para BL %i)]\n" RESET, nivel_punteros, indice, ptr, ptr, nivel_punteros);

                if (bwrite(ptr_ant, buffer)){ //salvamos en el dispositivo el buffer de punteros modificado
                        fprintf(stderr, RED "Error al escribir en el buffer\n" RESET);
                        return FALLO;
                    
                }
            }
        }
    }
        return ptr;


}










