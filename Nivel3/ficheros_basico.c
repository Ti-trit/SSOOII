#include "ficheros_basico.h"

#define byte 8
#define posLibre   0
//Metodos
int tamMB(unsigned int nbloques);
int tamAI(unsigned int ninodos);
int initSB(unsigned int nbloques, unsigned int ninodos);
int initMB(); 
int initAI();



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
            if (bufferAux[i]!=255){
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
        //devolver el nª del bloque reservado
        return nbloque;


    }else{//No hay bloques libres
        fprintf(stderr, RED"No hay bloques liberar!\n"RESET);
        return FALLO;
    }
    


}
