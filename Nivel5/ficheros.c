
#include "ficheros.h"

/**
 * @brief   el contenido de un buffer de tamaño nbytes en un fichero/directorio
 * en la posición offset.
 * @param   ninodo  posición del inodo en el AI
 * @param   buf_original  contiene el contenido que queremos escribir
 * @param   offset  posición de escritura inicial en bytes lógicos
 * @param   nbytes  número de bytes que queremos escribir
 * @return   cantidad de bytes escritos
 **/


  int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){
    struct inodo inodo;
 if (leer_inodo(ninodo, &inodo)==FALLO){
                fprintf(stderr, RED"mi_write_f: Error al leer el inodo\n" RESET);
                return FALLO;
        }
        //Verificamos que tenemos permisos de escritura
  if ((inodo.permisos&2)!=2){
            fprintf(stderr, RED "mi_write_f: No hay permisos de escritura\n" RESET);
       return FALLO;
        }

    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    unsigned int desp1 = offset % BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE; 
    char unsigned buf_bloque[BLOCKSIZE];

    int bytesEscritos = 0, auxBytesEscritos = 0;

    unsigned int nbfisico = traducir_bloque_inodo(&inodo, primerBL, 1);
    if (nbfisico == FALLO){
        fprintf(stderr, RED "mi_write_f: Error en traducir_bloque_inodo().\n" RESET);
        return FALLO;
    }
    //alamcenar el contenido el bloque fisico en el buffer
    if (bread(nbfisico, buf_bloque) == FALLO){
        fprintf(stderr, RED "mi_write_f: Error al leer el bloque fisico \n" RESET);
        return FALLO;
    }
    //1r PASO: primer bloque
    
    unsigned int bytesPrimerbl;
    if(primerBL == ultimoBL){//cuando solo hay que escribir un bloque
        bytesPrimerbl = nbytes;
    }else{ // primerBL < ultimoBL
        bytesPrimerbl = BLOCKSIZE - desp1;
    }
        //copiamos el contenido del buffer original al buffer del bloque desplazado
    if(memcpy(buf_bloque + desp1, buf_original, bytesPrimerbl) == NULL){
        fprintf(stderr, RED "mi_write_f: Error\n" RESET);
        return FALLO;
    }
        //escribimos el bloque
    auxBytesEscritos = bwrite(nbfisico, buf_bloque);
    if (auxBytesEscritos == FALLO) {
        fprintf(stderr, RED "mi_write_f: Error escribiendo bloques\n" RESET);
        return FALLO;
    }
    //acumulamos la cantidad de bytes escritos
    if(primerBL == ultimoBL){
        bytesEscritos += nbytes; 
    }else{ 
        bytesEscritos += auxBytesEscritos - desp1;
    }
    //2o PASO: Bloques intermedios (completos)
    if (primerBL < ultimoBL){
        for (int bloque = 1 + primerBL; bloque < ultimoBL; bloque++){
            nbfisico = traducir_bloque_inodo(&inodo, bloque, 1);
            if (nbfisico == FALLO){
                fprintf(stderr, RED "mi_write_f: Error en traducir_bloque_inodo() \n" RESET);
                return FALLO;
            }
            auxBytesEscritos = bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (bloque - primerBL - 1) * BLOCKSIZE);
            if (auxBytesEscritos<0)
            {
               fprintf(stderr, RED "Error al escribir en el bloque\n"RESET);
                return FALLO;
            }
            //BLOCKSIZE            
            bytesEscritos += auxBytesEscritos; 
        }

        // 3r PASO: último bloque a escribir 
        nbfisico = traducir_bloque_inodo(&inodo, ultimoBL, 1);
        if (nbfisico == FALLO)
        {
             fprintf(stderr, RED "Error en traducir_bloque_inodo()"RESET);
            return FALLO;
        }
        if (bread(nbfisico, buf_bloque) == FALLO)
        {
            fprintf(stderr, RED "Error al leer el bloque\n"RESET);
            return FALLO;
        }
        if(memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1) == NULL){
            fprintf(stderr, RED "mi_write_f: Error\n" RESET);
            return FALLO;
        }
        auxBytesEscritos = bwrite(nbfisico, buf_bloque);
        if (auxBytesEscritos == FALLO) {
             fprintf(stderr, RED "Error al escribir en el bloque en mi_write_f\n"RESET);
            return FALLO;
        }
        bytesEscritos += desp2 + 1;
    }

     //actaulizamos los datos del inodo si hemos escrito más allá del final del fichero

    if ((offset + nbytes) > inodo.tamEnBytesLog) { 
        inodo.tamEnBytesLog = nbytes + offset;
        inodo.ctime = time(NULL);
    }
    inodo.mtime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == FALLO){ // escribir inodo actualizado
        fprintf(stderr, RED "Error escribiendo inodo %i" RESET, ninodo);
        return FALLO;
    }
    return nbytes==bytesEscritos?  bytesEscritos: FALLO;
}


/**
 * @brief   la información de un fichero/directorio y la almacena en un buffer.
 * @param   ninodo  posicion del inodo en el AI
 * @param   buf_original    almacenará el contenido leido
 * @param   offset posicion de lectura inicial en bytes logicos
 * @param   nbytes  numero de bytes que queremos leer
 */

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){
    struct inodo inodo;
     if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "mi_read_f: Error al leer el inodo" RESET);
        return FALLO;
    }
    // Comprobamos si tenemos permisos de escritura
    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr, RED "mi_read_f: No hay permisos de lectura\n" RESET);
        return FALLO;
    }

    unsigned int bytesLeidos = 0;
    // comprobación del EOF
    if (offset >= inodo.tamEnBytesLog){
        // no podemos leer nada
        return bytesLeidos;
    }
    if ((offset + nbytes) >= inodo.tamEnBytesLog){
        // pretende leer más allá de EOF
        nbytes = inodo.tamEnBytesLog - offset;
   // leemos sólo los bytes que podemos desde el offset hasta EOF

    }
    char unsigned buf_bloque[BLOCKSIZE];
    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    unsigned int desp1 = offset % BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    int nbfisico = traducir_bloque_inodo(&inodo, primerBL, 0);
    unsigned int bytesPrimerBl;

    if(primerBL == ultimoBL){
        bytesPrimerBl = nbytes;
    }else{ // primerBL < ultimoBL
        bytesPrimerBl = BLOCKSIZE - desp1;
    }

    if (nbfisico != FALLO){

        if (bread(nbfisico, buf_bloque) == FALLO) {
                fprintf(stderr, RED "Error al leer el bloque fisico en mi_read_f()\n" RESET);
            return FALLO;
        }
        if(memcpy(buf_original, buf_bloque + desp1, bytesPrimerBl) == NULL){
            fprintf(stderr, RED "mi_read_f: Error\n" RESET);
            return FALLO;
        }
    }

    bytesLeidos = bytesPrimerBl;
    //cuando hay que leer más de un bloque
    if (primerBL < ultimoBL){
        for (int bloque = 1 + primerBL; bloque < ultimoBL; bloque++){
            nbfisico = traducir_bloque_inodo(&inodo, bloque, 0);
            bytesLeidos += BLOCKSIZE;
            if (nbfisico == FALLO)
            {
                continue;
            }
            if (bread(nbfisico, buf_bloque) == FALLO)
            {
                fprintf(stderr, RED "Error al leer el bloque fisico en mi_read_f()\n" RESET);
                return FALLO;
            }
            if ((memcpy(buf_original + (BLOCKSIZE - desp1) + (bloque - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE)) == NULL){
                fprintf(stderr, RED "mi_read_f: Error en memcpy\n" RESET);
                return FALLO;
            }
        }

        // último bloque
        nbfisico = traducir_bloque_inodo(&inodo, ultimoBL, 0);
        if (nbfisico != FALLO)
        {
            if (bread(nbfisico, buf_bloque) == FALLO)
            {
            fprintf(stderr, RED "Error al leer el bloque fisico en mi_read_f()\n" RESET);
                return FALLO;
            }
            if(memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 + 1) == NULL){
                fprintf(stderr, RED "mi_read_f: Error en memcpy()\n" RESET);
                return FALLO;
            }
        }
        bytesLeidos += (desp2 + 1);
    }

    if (leer_inodo(ninodo, &inodo) == FALLO){
        fprintf(stderr, "Error en leer_inodo en mi_read_f().\n");
        return FALLO;
    }
    if ((offset + nbytes) > inodo.tamEnBytesLog) { // si hemos escrito más allá del final del fichero (bytesLeidos > inodo.tamEnBytesLog) ???
        inodo.tamEnBytesLog = nbytes + offset;
        inodo.ctime = time(NULL);
    }

    inodo.atime = time(NULL);
    //guardar los cambios del inodo
    if (escribir_inodo(ninodo, &inodo) == FALLO){ 
        fprintf(stderr, "Error al salvar el inodo()\n" RESET);
        return FALLO;
    }
    
    return nbytes==bytesLeidos?  bytesLeidos: FALLO;
}


/**
 * Devuelve la metainformación de un fichero/directorio correspondiente
 * al inodo pasado.
 * @param   ninodo  posición del inodo en el AI
 * @param   p_stat  strcut semejante al inodo pero sin los puntero ni el padding
 */
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{

    struct inodo inodo;
    // leemos el inodo para obtener sus datos
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        perror("Errir en leer_inodo()");
        return FALLO;
    }

    // asignación de los datos del inodo a p_stat
    p_stat->tipo = inodo.tipo;
    p_stat->atime = inodo.atime;
    p_stat->ctime = inodo.ctime;
    p_stat->mtime = inodo.mtime;
    p_stat->nlinks = inodo.nlinks;
    p_stat->permisos = inodo.permisos;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;

    // printf ("%s %s %i %i %t %i", p_stat->tipo, p_stat->permisos, p_stat->nlinks, p_stat->tamEnBytesLog, p_stat->atime, p_stat->numBloquesOcupados);
    return EXITO;
}

/**
 * Cambia los permisos de un fichero/directorio (correspondiente al nº de inodo pasado como argumento, ninodo)
 * @param   ninodo    nº inodo que cambairemos sus permisos
 * @param   permisos  permisos nuevos
 * @return  EXITO o FALLO
 */
int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    struct inodo inodo;
    // leer el inodo para obtener los permisos
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "Error al leer el inodo en mi_chmod_f()\n" RESET);
        return FALLO;
    }
    // cambiar los permisos
    inodo.permisos = permisos;
    // actualizar el tiempo de ultima modificación del inodo
    inodo.ctime = time(NULL);

    // guardar las modificaciones del inodo
    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "Error en escribir_inodo : mi_chmod_f()\n" RESET);
        return FALLO;
    }
    return EXITO;
}
