
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


    int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    struct inodo inodo;

    if (leer_inodo(ninodo, &inodo)==FALLO){
                fprintf(stderr, RED"Error al leer el inodo" RESET);
                return FALLO;
        }
        //Verificamos que tenemos permisos de escritura
  if ((inodo.permisos&2)!=2){
            fprintf(stderr, RED "No hay permisos de escritura\n" RESET);
       return FALLO;
        }

    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    char unsigned buf_bloque[BLOCKSIZE];
    int bytesescritos = 0;
    int aux_bescritos  = 0;
    int nbfisico = traducir_bloque_inodo(&inodo, primerBL, 1);
    if (nbfisico == FALLO)
    {
          fprintf(stderr, RED "Error en traducir_bloque_inodo().\n"RESET);
            return FALLO;
    }

    if (bread(nbfisico, buf_bloque) == FALLO)
    {
        fprintf(stderr, "Error mi_write_f bread\n");
        return FALLO;
    }


    if (primerBL == ultimoBL) // Cuando solo se escribe sobre un bloque
    {
        //almacenamos el contenido de nbfisoc en buf_bloque
        memcpy(buf_bloque + desp1, buf_original, nbytes);

        aux_bescritos  = bwrite(nbfisico, buf_bloque);
        if (aux_bescritos  == FALLO)
        {
            fprintf(stderr, "Error mi_write_f bwrite\n");
            return FALLO;
        }
        bytesescritos += nbytes;
    }
    else if (primerBL < ultimoBL) // cabe más de un bloque
    {
        // PRIMER PASO: Primer bloque lógico

        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);

        aux_bescritos  = bwrite(nbfisico, buf_bloque);
        if (aux_bescritos  == FALLO)
        {
             fprintf(stderr, RED "Error al escribir en el bloque\n"RESET);
            return FALLO;
        }
                //acualizamos los bytes escritos
        bytesescritos += (aux_bescritos  - desp1);

        // SEGUNDO PASO: Bloques lógicos intermedios


        for (int i = primerBL + 1; i < ultimoBL; i++)
        {
            nbfisico = traducir_bloque_inodo(&inodo, i, 1);
            if (nbfisico == FALLO)
            {
                fprintf(stderr, "Error mi_write_f traducir_bloque_inodo\n");
                return FALLO;
            }

            aux_bescritos  = bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE);
            if (aux_bescritos <0)
            {
               fprintf(stderr, RED "Error al escribir en el bloque\n"RESET);
                return FALLO;
            }
            //sumamos directamente porque son bloques completos
            bytesescritos += aux_bescritos ;
        }

        // TERCER PASO: Último bloque lógico


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

        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);

        aux_bescritos  = bwrite(nbfisico, buf_bloque);
        if (aux_bescritos  == FALLO)
        {
             fprintf(stderr, RED "Error al escribir en el bloque\n"RESET);
            return FALLO;
        }

        bytesescritos += desp2 + 1;
    }

    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, "Error mi_write_f leer_inodo\n");
        return FALLO;
    }

     //actaulizamos solo si hemos escrito más allá del final del fichero

    if (inodo.tamEnBytesLog < (offset + nbytes))
    {
        inodo.tamEnBytesLog = nbytes + offset;
        inodo.ctime = time(NULL);
    }

    inodo.mtime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, "Error mi_write_f escribir_inodo\n");
        return FALLO;
    }

    if (nbytes == bytesescritos)
    {
        return bytesescritos;
    }
    else
    {
        return FALLO;
    }
}

/**
 * @brief   la información de un fichero/directorio y la almacena en un buffer.
 * @param   ninodo  posicion del inodo en el AI
 * @param   buf_original    almacenará el contenido leido
 * @param   offset posicion de lectura inicial en bytes logicos
 * @param   nbytes  numero de bytes que queremos leer
 */


int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    struct inodo inodo;

    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "Error al leer el inodo" RESET);
        return FALLO;
    }
    // Comprobamos si tenemos permisos de escritura
    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr, RED "No hay permisos de lectura\n" RESET);
        return FALLO;
    }

    int leidos = 0;
    int auxbleidos = 0;
    // Comprobamos si podemos leer
    if (offset >= inodo.tamEnBytesLog)
    {
        // no podemos leer nada
        return leidos;
    }

    if ((offset + nbytes) >= inodo.tamEnBytesLog)
    {
        // pretende leer más allá de EOF
        nbytes = inodo.tamEnBytesLog - offset;
        // leemos sólo los bytes que podemos desde el offset hasta EOF

    }

    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    char unsigned buf_bloque[BLOCKSIZE];

    int nbfisico = traducir_bloque_inodo(&inodo, primerBL, 0);
        // PRIMER CASO

    if (primerBL == ultimoBL)
    {
        if (nbfisico != FALLO)
        {
            auxbleidos = bread(nbfisico, buf_bloque);
            if (auxbleidos == FALLO)
            {
                fprintf(stderr, RED "Error al leer el bloque fisico en mi_read_f()\n" RESET);
                return FALLO;
            }
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }

        leidos = nbytes;
    }
        // SEGUNDO CASO

    else if (primerBL < ultimoBL)
    {
        // Primer bloque logico
         if (nbfisico != FALLO)
        {
            auxbleidos = bread(nbfisico, buf_bloque);
            if (auxbleidos <0)
            {
                fprintf(stderr, RED "Error al leer el bloque fisico en mi_read_f()\n" RESET);
            return FALLO;
            }
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }

        leidos = BLOCKSIZE - desp1;

        // Bloques intermedios
        for (int i = primerBL + 1; i < ultimoBL; i++)
        {
            nbfisico = traducir_bloque_inodo(&inodo, i, 0);
            if (nbfisico != FALLO)
            {
                auxbleidos = bread(nbfisico, buf_bloque);
                if (auxbleidos == FALLO)
                {
                fprintf(stderr, RED "Error al leer el bloque fisico en mi_read_f()\n" RESET);
                    return FALLO;
                }
                memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
            }

            leidos += BLOCKSIZE;
        }

        //  Último bloque 
        nbfisico = traducir_bloque_inodo(&inodo, ultimoBL, 0);
        if (nbfisico != FALLO)
        {
            auxbleidos = bread(nbfisico, buf_bloque);
            if (auxbleidos == FALLO)
            {
            fprintf(stderr, RED "Error al leer el bloque fisico en mi_read_f()\n" RESET);
                return FALLO;
            }
            memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 + 1);
        }

        leidos += (desp2 + 1);
    }

    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, "Error en leer_inodo en mi_read_f().\n");
        return FALLO;
    }

    inodo.atime = time(NULL);
        //guardar los cambios del inodo
    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, "Error al salvar el inodo()\n" RESET);
        return FALLO;
    }

    if (nbytes == leidos)
    {
        return leidos;
    }
    else
    {
        return FALLO;
    }
}

/**
 * Devuelve la metainformación de un fichero/directorio correspondiente
 * al inodo pasado.
 * @param   ninodo  posición del inodo en el AI
 * @param   p_stat  strcut semejante al inodo pero sin los puntero ni el padding
 */
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    // tipo, permisos, cantidad de enlaces de entradas en directorio,
    //  tamaño en bytes lógicos, timestamps y cantidad de bloques ocupados en la zona de datos
    struct inodo inodo;
    // leemos el inodo para obtener sus datos
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        perror("Errir en leer_inodo()");
        return FALLO;
    }

    // asignación de los datos del inodo a p_stat
    p_stat->atime = inodo.atime;
    p_stat->ctime = inodo.ctime;
    p_stat->mtime = inodo.mtime;
    p_stat->nlinks = inodo.nlinks;
    p_stat->permisos = inodo.permisos;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->tipo = inodo.tipo;
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