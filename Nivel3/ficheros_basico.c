#include "ficheros_basico.h"




/**
 * Calcula el tamaño en bloques para el mapa de bits.
 * @param   nbloques
 * @return  tamaño de bloques para el MB
*/

int tamMB(unsigned int nbloques){

        int tambloquesMb= (nbloques/8)/BLOCKSIZE;
        if ((nbloques/8) % BLOCKSIZE!=0){
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


int initSB(unsigned int nbloques, unsigned int ninodos){

        struct superbloque SB;
        //Datos del mapa de bits
        SB.posPrimerBloqueMB= posSB+tamSB; // posSB= 0, tamSB= 1
        SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;

        //Datos del array de inodos
        SB.posPrimerBloqueAI= SB.posUltimoBloqueMB+1;
        SB.posUltimoBloqueAI=SB.posPrimerBloqueAI+tamAI(ninodos)-1;

        //Datos del bloque de datos
        SB.posPrimerBloqueDatos= SB.posUltimoBloqueAI+1;
        SB.posUltimoBloqueDatos=nbloques-1;
        
        //Datos de los inodos y bloques
        SB.posPrimerInodoLibre=0;
        //Posición del primer inodo libre
        SB.posInodoRaiz=0;
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
    memset(bufferMB, 0,BLOCKSIZE);

    //leer el superbloque para obtener nº de bloques y inodos
    if (bread(posSB, &SB)<0){
        fprintf(stderr, RED "Error al leer el SB (initMB())\n"RESET);
        return FALLO;
    }

    int bloquesMetadatos = tamSB + tamMB(SB.totBloques)+ tamAI(SB.totInodos);
      

      if (bloquesMetadatos % 8 == 0)
    {
        for (int i = 0; i < bloquesMetadatos / 8; i++)
        {
            bufferMB[i] = 255;
        }
    }
    else // Si no ocupan bloques exactos tendremos que hacer un desplazamiento
    {
        for (int i = 0; i <= bloquesMetadatos / 8; i++)
        {
            bufferMB[i] = 255;
        }
            bufferMB[bloquesMetadatos / 8] = bufferMB[bloquesMetadatos / 8] << (8 - (bloquesMetadatos % 8));
    }
     

     if (bwrite(SB.posPrimerBloqueMB, bufferMB) == FALLO)
    {
        perror("Error initMB bwrite (MB)");
        return FALLO;
    }


            //escribir el MB actualizado
    if (bwrite(SB.posPrimerBloqueMB, bufferMB) == FALLO)
    {
        fprintf(stderr, RED"Error initMB bwrite (MB)"RESET);
        return FALLO;
    }
     //restar estos bloques de la cantidad de bloques libres

             SB.cantBloquesLibres -= bloquesMetadatos;

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
    fprintf(stderr, RED "Error al leer el SB en initAI()\n"RESET);
            return FALLO;
        }

    int contInodos = SB.posPrimerInodoLibre + 1;
    int fi = 0;

    //Leemos cada bloque
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI && fi==0; i++){
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
                fi = 1;
                break;
            }
        }
        if (bwrite(i,inodos) == -1){
            perror(RED"Error en escribir los inodos"RESET);
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
   
    
    //leemos el superbloque para obtenir la posición del MB
    if (bread(posSB, &SB)==FALLO){
        fprintf(stderr, RED "Error al leer el bloque\n"RESET);
        return FALLO;
    }
    int posbyte = nbloque/8;
    int posbit = nbloque%8;
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

    if (bwrite(nbloqueabs, bufferMB)<0){
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
  
    
    //leemos el superbloque para obtenir la posición del MB
    if (bread(posSB, &SB)==FALLO){
        fprintf(stderr, RED "Error al leer el bloque\n"RESET);
        return FALLO;
    }
      int posbyte = nbloque/8;
    int posbit = nbloque%8;
    //posicion del bloque que contiene el byte
    int nbloqueMB = posbyte/BLOCKSIZE;
    //posición absoluta del bloque que buscamos
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];

    if (bread(nbloqueabs, bufferMB)==FALLO){
        perror( RED "Error al leer el bloque\n"RESET);
        return FALLO;
    }
    //obtener la posición de este byte dentro del bufferMB
    posbyte = posbyte % BLOCKSIZE;

    unsigned char mascara = 128; // 10000000
    mascara>>= posbit; // Desplazamiento del bit "posbit" veces a la derecha.
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
        unsigned char bufferMB[BLOCKSIZE];
        unsigned char bufferAux [BLOCKSIZE];
        memset(bufferAux, 255, BLOCKSIZE);//poner los bits de bufferAux a 1

        int nbloqueMB = SB.posPrimerBloqueMB;

        while (nbloqueMB<=SB.posUltimoBloqueMB){
        if (bread(nbloqueMB , bufferMB)<0){
            perror(RED "Error al leer el bloque\n. "RESET);
            return FALLO;
        }
       if (memcmp(bufferMB, bufferAux, BLOCKSIZE)!=0){ //bloque libre
            break;

       }
        nbloqueMB++;
        }
        //ahora buefferMB contiene el byte con algún bit a 0
        //localizamos de cúal byte se trata
        int posbyte=0;
        for (int i = 0; i<BLOCKSIZE; i++){
            if (bufferMB[i]!=255){
                posbyte=i;
                break;
            }
        }
    
      //localisamos cual bit está a 0 del byte (posbyte)
            unsigned char mascara = 128; //10000000
            int posbit = 0;
            while (bufferMB[posbyte] & mascara){//operador AND para bits
                bufferMB[posbyte] <<=1;  //desplazamiento hacia la izequierda de bits
                posbit++;
            }

        //nº de bloque físico a reservar
        int nbloque = ((nbloqueMB-SB.posPrimerBloqueMB)*BLOCKSIZE+posbyte)*8 + posbit;
        printf ("nbloque calculado: %i \n", nbloque);
        //escrbimos el bit a 1 para indicar que está reservado
        if (escribir_bit(nbloque,1 )<0){
            fprintf(stderr, RED"Error al escribir el bit 1 para reservar el bloque\n"RESET);
            return FALLO;
        }
        //decrementamos la cantidad de bloques libres
        SB.cantBloquesLibres--;
        
        //limpiamos ese bloque en la zona de datos
        int posVirtual= nbloque+SB.posPrimerBloqueDatos-1;
        memset(bufferAux, 0, BLOCKSIZE);
        if (bwrite(posVirtual, bufferAux)<0){
            fprintf(stderr, RED"Error al limpiar el bloque reservado en al zona de datos\n"RESET);
            return FALLO;
        }

    if (bwrite(posSB, &SB)<0){
        fprintf(stderr, RED"Error al guardar los cambios en el SB  \n"RESET);
        return FALLO;
    }
      
        //devolver el nª del bloque reservado
        printf("\n nbloque: %i \n ", nbloque);
       // printf("\n posvirtual: %i \n ", posVirtual);
        return nbloque;


    }else{//No hay bloques libres
        fprintf(stderr, RED"No hay bloques libres.!\n"RESET);
        return FALLO;
    }
}


int liberar_bloque(unsigned int nbloque){
    struct superbloque SB;
    if (bread(posSB, &SB)==FALLO){
        fprintf(stderr, RED "Error al leer el superbloque\n"RESET);
        return FALLO;
    }
    //Escribimos el bit a 0 para indicar que esta libre
    if (escribir_bit(nbloque, 0)<0){
        fprintf(stderr, RED "Error al escribir 0 para liberar el bloque\n"RESET);
        return FALLO;
    }
    SB.cantBloquesLibres++;
//guardar el SB modificado
         if (bwrite(posSB, &SB)<0){
        fprintf(stderr, RED"Error al guardar los cambios en el SB  \n"RESET);
        return FALLO;
    }
    

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
    if (bread(posSB, &SB)==FALLO){
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

    inodos[ninodo %(nInodosPorBloque)]=*inodo;
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
    if (bread(posSB, &SB)==FALLO){
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
        perror( RED"Error al leer el inodo\n" RESET);
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
        if (bread(posSB, &SB)==FALLO){
                fprintf(stderr, RED "Error al leer el superbloque\n"RESET);
                return FALLO;
        }

        if (SB.cantInodosLibres==0){
                fprintf(stderr, RED "No hay inodos libres para reservar\n"RESET);
                return FALLO;
        }

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
             /*  for (int i = 0; i<12; i++){
                    if (i<3){
                        inodoReservado.punterosDirectos[i]=0;
                    }
                    inodoReservado.punterosIndirectos[i]=0;
                }*/

                                for (int i = 0; i < 12; i++) {
                        inodoReservado.punterosDirectos[i] = 0;
                    }
                    for (int i = 0; i < 3; i++) {
                        inodoReservado.punterosIndirectos[i] = 0;
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



