
#include "ficheros.h"

/**
 * Devuelve la metainformación de un fichero/directorio correspondiente 
 * al inodo pasado.
 * @param   ninodo  posición del inodo en el AI
 * @param   p_stat  strcut semejante al inodo pero sin los puntero ni el padding
*/
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat){
    //tipo, permisos, cantidad de enlaces de entradas en directorio,
    // tamaño en bytes lógicos, timestamps y cantidad de bloques ocupados en la zona de datos
    struct inodo inodo;
    //leemos el inodo para obtener sus datos
    if (leer_inodo(ninodo,&inodo)==FALLO){
            perror("Errir en leer_inodo()");
            return FALLO;
    }

   //asignación de los datos del inodo a p_stat
    p_stat->atime=inodo.atime;
    p_stat->ctime=inodo.ctime;
    p_stat->mtime= inodo.mtime;
    p_stat->nlinks=inodo.nlinks;
    p_stat->numBloquesOcupados= inodo.numBloquesOcupados;
    p_stat->permisos=inodo.permisos;
    p_stat->tamEnBytesLog=inodo.tamEnBytesLog;
    p_stat->tipo= inodo.tipo;

 //printf ("%s %s %i %i %t %i", p_stat->tipo, p_stat->permisos, p_stat->nlinks, p_stat->tamEnBytesLog, p_stat->atime, p_stat->numBloquesOcupados);
    return EXITO;


}

/**
 * Cambia los permisos de un fichero/directorio (correspondiente al nº de inodo pasado como argumento, ninodo) 
 * @param   ninodo    nº inodo que cambairemos sus permisos
 * @param   permisos  permisos nuevos
 * @return  EXITO o FALLO
*/
int mi_chmod_f(unsigned int ninodo, unsigned char permisos){
        struct inodo inodo; 
        //leer el inodo para obtener los permisos
        if (leer_inodo(ninodo, &inodo)==FALLO){
              fprintf(stderr,RED"Error al leer el inodo en mi_chmod_f()\n"RESET);  
              return FALLO;
        }
        //cambiar los permisos
        inodo.permisos= permisos;
        //actualizar el tiempo de ultima modificación del inodo
        inodo.ctime= time(NULL);

        //guardar las modificaciones del inodo
        if (escribir_inodo(ninodo, &inodo)==FALLO){
            fprintf(stderr, RED"Error en escribir_inodo : mi_chmod_f()\n"RESET);    
            return FALLO;
        }
        return EXITO;
}
