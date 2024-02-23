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



