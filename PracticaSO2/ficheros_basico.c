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
        if ((nbloques/byte) % BLOCKSIZE==0){
            //nbloques es multiple de BLOCKSIZE
            return tambloquesMb;
        }else{
            //el ultimo bloque no es de BlOCKSIZE
            return tambloquesMb+1;
        }

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
        SB.posUltimoBloqueMB = SB.posPrimerBloqueMB+tamMB(nbloques)-1;

        //Datos del array de inodos
        SB.posPrimerBloqueAI= SB.posPrimerBloqueMB+1;
        SB.posUltimoBloqueAI=SB.posPrimerBloqueAI+tamAI(ninodos)-1;

        //Datos del bloque de datos
        SB.posPrimerBloqueDatos= SB.posUltimoBloqueAI+1;
        SB.posUltimoBloqueDatos=nbloques-1;
        
        //Datos de los inodos y bloques
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
       }else{
        return EXITO;
       }

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
    if (bread(posSB, &SB)<1){
        return FALLO;
    }

    int bloquesMetadatos = tamSB + tamMB(SB.totBloques)+ tamAI(SB.totInodos);
    int bitsMetadatos = bloquesMetadatos/8;
       for (int i = 0; i<bloquesMetadatos/8; i++){
            bufferMB[i]=255; //rellenar con 1
            //no caben en un bloque rellenamos el ultimo bloque manualmente

      if (bloquesMetadatos%8!=0){
           // bufferMB[(bitsMetadatos/8)]=256-(int)pow(2.0,(double)(bloquesMetadatos%8));
                bufferMB[bitsMetadatos/8]=(256 - (1 << (bitsMetadatos%8)));
      }

    }

    for(int i = SB.posPrimerBloqueMB; i<SB.posPrimerBloqueMB; i++){
       if (bwrite(i, bufferMB)<1){
        fprintf(stderr, "Error al rellenar el mapa de bits\n"RESET);
        return FALLO;
       }
    }
        //escribir el MB actualizado
    if (bwrite(SB.posPrimerBloqueMB, bufferMB)<1){
        fprintf(stderr, RED"Error al escribir el bloque %d \n"RESET, SB.posPrimerBloqueMB);
        return FALLO;
    }

    //guardar los cambios en el superbloque
    if (bwrite(posSB, &SB)<1){
        fprintf(stderr, RED"Error al guardar los cambios en el SB  \n"RESET);
        return FALLO;
    }
    //restar estos bloques de la cantidad de bloques libres
   SB.cantBloquesLibres= SB.cantBloquesLibres-bloquesMetadatos;
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
