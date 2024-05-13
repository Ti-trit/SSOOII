#include "directorios.h"
/**
 * @brief Función que separa una cadena(que empieza por /) en dos partes,  inicial y final.
 * @param   camino    cadena a separar
 * @param   inicial   parte inicial obtenido de camino
 * @param   final     resta de la cadena camino
 * @param   tipo      tipo del camino
 * @return            EXITO o FALLO
 */
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{
    if (camino[0] != '/')
    {
        // fprintf(stderr, RED"extraer_camino:Error, el camino no contiene / al inicio\n" RESET);
        // return FALLO;
        return ERROR_CAMINO_INCORRECTO;
    }

    const char *segundo_slash = strchr(camino + 1, '/');
    if (segundo_slash != NULL)
    {
        // Extraer la posicion hasta el segundo '/'
        int len = segundo_slash - (camino + 1);

        if (strncpy(inicial, camino + 1, len) == NULL)
        {
            perror("extraer_camino:Error strncpy()");
            return FALLO;
        }
        inicial[len] = '\0';

        // Resto del camino
        strcpy(final, segundo_slash);
        *tipo = 'd'; // Es un directorio porque hay un segundo '/'
    }
    else
    {
        // No hay segunco '/', por lo tanto es un fichero
        if (strcpy(inicial, camino + 1) == NULL)
        {
            perror("extraer_camino:Error strncpy()");
            return FALLO;
        }

        final[0] = '\0';
        *tipo = 'f';
    }

    return EXITO;
}

void mostrar_error_buscar_entrada(int error)
{
    // fprintf(stderr, "Error: %d\n", error);
    switch (error)
    {
    case -2:
        fprintf(stderr, RED "Error: Camino incorrecto.\n" RESET);
        break;
    case -3:
        fprintf(stderr, RED "Error: Permiso denegado de lectura.\n" RESET);
        break;
    case -4:
        fprintf(stderr, RED "Error: No existe el archivo o el directorio.\n" RESET);
        break;
    case -5:
        fprintf(stderr, RED "Error: No existe algún directorio intermedio.\n" RESET);
        break;
    case -6:
        fprintf(stderr, RED "Error: Permiso denegado de escritura.\n" RESET);
        break;
    case -7:
        fprintf(stderr, RED "Error: El archivo ya existe.\n" RESET);
        break;
    case -8:
        fprintf(stderr, RED "Error: No es un directorio.\n" RESET);
        break;
    }
}

/**
 * @brief Función que busca una determinada entrada entre todas
 * las entradas del inodo correspondiente a su directorio padre.
 * @param   camino_parcial  camino del que extraemos la entrada
 * @param   p_inodo_dir     nº del inodo del directorio padre
 * @param   p_inodo     nº del inodo asociada a la entrada de busqueda
 * @param   p_entrada   nº de entrada dentro del inodo p_inodo_dir
 * @param   reservar
 * @param   permisos
 * @return  EXITO O FALLO
 */

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;
    struct superbloque SB;

    // leemos el superbloque
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "buscar_entrada: Error en bread()\n" RESET);
        return FALLO;
    }

    if (strcmp(camino_parcial, "/") == 0)
    { // directorio raiz
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return EXITO;
    }
    memset(inicial, 0, sizeof(inicial));
    memset(final, 0, strlen(camino_parcial));
    memset(entrada.nombre, 0, sizeof(entrada.nombre));

    if (extraer_camino(camino_parcial, inicial, final, &tipo) < 0)
    {
        return ERROR_CAMINO_INCORRECTO;
    }

#if DEBUGN7
    fprintf(stderr, GRAY "[buscar_entrada()→ inicial: %s, final: %s, reservar: %i]\n" RESET, inicial, final, reservar);
#endif

    if (leer_inodo(*p_inodo_dir, &inodo_dir) == FALLO)
    {
        fprintf(stderr, RED "buscar_entrada; Error en leer_inodo %ls\n" RESET, p_inodo_dir);
        return FALLO;
    }

    // verificamos que tiene permisos de lectura
    if ((inodo_dir.permisos & 4) != 4)
    {
        return ERROR_PERMISO_LECTURA;
    }
    struct entrada bufferEntradas[BLOCKSIZE / sizeof(entrada)];

    // inicializar el bufferEntradas de lectura con 0s
    if (memset(bufferEntradas, 0, BLOCKSIZE) == NULL)
    {
        fprintf(stderr, RED "buscar_entrada: Error en el memset()\n" RESET);
        return FALLO;
    }

    // calculamos la cantidad de entrada que contiene el inodo
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(entrada);
    num_entrada_inodo = 0; // nº de entrada inicial

    if (cant_entradas_inodo > 0)
    {
        // leemos las entradas en el buffer de entradas
        if (mi_read_f(*p_inodo_dir, bufferEntradas, 0, BLOCKSIZE) == FALLO)
        {
            fprintf(stderr, RED "buscar_entrada: Error al leer el bloque de entrada\n" RESET);
            return FALLO;
        }
        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(bufferEntradas[num_entrada_inodo].nombre, inicial) != 0))
        {
            num_entrada_inodo++;
            // leer siguiente entrada
        }
    }
    // hasta Aqui
if ((!(num_entrada_inodo < BLOCKSIZE/sizeof(struct entrada)) || (strcmp(inicial, bufferEntradas[num_entrada_inodo].nombre) != 0)) && (num_entrada_inodo == cant_entradas_inodo)) { // La entrada no existe

//     if ((strcmp(inicial, bufferEntradas[num_entrada_inodo].nombre) != 0) && num_entrada_inodo == cant_entradas_inodo){
        switch (reservar)
        {
        case 0:
            // modo consulta
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        case 1:
            // modo escritura

            // si es fichero no permitir escritua
            if (inodo_dir.tipo == 'f')
            {
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }

            // si es directorio comprobamos que tiene permisos de escritura

            if ((inodo_dir.permisos & 2) != 2)
            {
                return ERROR_PERMISO_ESCRITURA;
            }

            // tenemos permisos de escritura

            // copiar *inicial en el nombre de la entrada
            if (strncpy(bufferEntradas[num_entrada_inodo].nombre, inicial, sizeof(entrada.nombre)) == NULL)
            {
                fprintf(stderr, RED "buscar_entrada: Error en strcpy()\n" RESET);
                return FALLO;
            }

            if (tipo == 'd')
            {
                if (strcmp(final, "/") != 0)
                {
                    return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                }

                // reservamos un inodo directorio y le asiganremos la entrada
                int ninodo = reservar_inodo('d', permisos);

                if (ninodo == FALLO)
                {
                    fprintf(stderr, RED "buscar_entrada: Error al reservar el inodo %i para el directorio\n" RESET, ninodo);
                    return FALLO;
                }
                bufferEntradas[num_entrada_inodo].ninodo = ninodo;
#if DEBUGN7
                fprintf(stderr, GRAY "[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n" RESET, bufferEntradas[num_entrada_inodo].ninodo, tipo, permisos, bufferEntradas[num_entrada_inodo].nombre);
#endif
            }
            else
            {
                // si es un fichero
                // reservamos un inodo como fichero y le asignamos a la entrada
                int ninodo = reservar_inodo('f', permisos);

                if (ninodo == FALLO)
                {
                    fprintf(stderr, RED "buscar_entrada: Error al reservar el inodo %i para el fichero \n" RESET, ninodo);
                    return FALLO;
                }
                bufferEntradas[num_entrada_inodo].ninodo = ninodo;
#if DEBUGN7
                fprintf(stderr, GRAY "[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n" RESET, bufferEntradas[num_entrada_inodo].ninodo, tipo, permisos, bufferEntradas[num_entrada_inodo].nombre);
#endif
            }

#if DEBUGN7
            fprintf(stderr, GRAY "[buscar_entrada()→ creada entrada: %s, %d] \n" RESET, inicial, bufferEntradas[num_entrada_inodo].ninodo);
#endif
            // escribir la entrada en el directorio padre
            if (mi_write_f(*p_inodo_dir, &bufferEntradas[num_entrada_inodo], num_entrada_inodo * sizeof(entrada), sizeof(entrada)) == FALLO)
            {
                int ninodo = bufferEntradas[num_entrada_inodo].ninodo;
                if (ninodo != FALLO)
                { // Si se había reservado un inodo para la entrada
                    // lo liberamos
                    if (liberar_inodo(ninodo) == FALLO)
                    {
                        fprintf(stderr, RED "buscar_entrada: Error al liberar el inodo %i reservado previamente\n" RESET, ninodo);
                        return FALLO;
                    }
                }

                return FALLO;
            }
        }
    }

    // hemos llegado al final del camino?
    if (strcmp(final, "") == 0 || strcmp(final, "/") == 0)
    {
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1))
        {
            // mode escritura y la entrada ya existe
            return ERROR_ENTRADA_YA_EXISTENTE;
        }

        *p_inodo = bufferEntradas[num_entrada_inodo].ninodo;
        *p_entrada = num_entrada_inodo;
        return EXITO;
    }
    else
    {
        *p_inodo_dir = bufferEntradas[num_entrada_inodo].ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
    return EXITO;
}


/**
 * @brief Pone el contenido del directorio en un buffer de memoria
 * @param  camino directorio
 * @param   buffer  para guardar el contenido del directorio
 * @param   tipo    tipo de directorio?
 * @return  nº de entradas leidas
 *
 */
int mi_dir(const char *camino, char *buffer, char tipo)
{

    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int err = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
    if (err < 0)
    {
        mostrar_error_buscar_entrada(err);
        return FALLO;
    }

    struct inodo inodo;
    if (leer_inodo(p_inodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "mi_dir: Error al leer el inodo %d" RESET, p_inodo);
        return FALLO;
    }
    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr, RED "No tiene permisos de lectura\n" RESET);
        return FALLO;
    }

    if (inodo.tipo != tipo)
    {
        fprintf(stderr, RED "Error: la sintaxis no concuerda con el tipo\n" RESET);
        return FALLO;
    }
    int Nentradas = inodo.tamEnBytesLog / sizeof(struct entrada);
    // struct entrada buffer_entradas[Nentradas];
    struct entrada Entradas[BLOCKSIZE / sizeof(struct entrada)];
    memset(Entradas, 0, sizeof(struct entrada));
    int offset = mi_read_f(p_inodo, &Entradas, 0, BLOCKSIZE);

    if (offset == FALLO)
    {
        fprintf(stderr, RED "mi_dir: Error en mi read()\n" RESET);
        return FALLO;
    }

    for (int i = 0; i < Nentradas; i++)
    {
        if (leer_inodo(Entradas[i % (BLOCKSIZE / sizeof(struct entrada))].ninodo, &inodo) == FALLO)
        {
            fprintf(stderr, RED "mi_dir: Error al leer el inodo " RESET);
            return FALLO;
        }
        // obtener la info de cada inodo

        if (inodo.tipo == 'd')
        {
            strcat(buffer, BLUE);
            strcat(buffer, "d");
        }
        else
        {
            strcat(buffer, GREEN);
            strcat(buffer, "f");
        }

        // añadir separación
        strcat(buffer, "\t");

        // permisos
        strcat(buffer, MAGENTA);
        strcat(buffer, (inodo.permisos & 4) ? "r" : "-");
        strcat(buffer, (inodo.permisos & 2) ? "w" : "-");
        strcat(buffer, (inodo.permisos & 1) ? "x" : "-");
        strcat(buffer, "\t"); //  separador

        // info acerca del tiempo
        struct tm *tm; // ver info: struct tm
        char tmp[64];
        tm = localtime(&inodo.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        strcat(buffer, tmp);
        strcat(buffer, "\t");

        // tamaño
        strcat(buffer, YELLOW);
        char tamEnBytesLog[10];
        sprintf(tamEnBytesLog, "%d", inodo.tamEnBytesLog);
        strcat(buffer, tamEnBytesLog);
        strcat(buffer, "\t");

        // nombre
        strcat(buffer, RED);
        char nombre[TAMNOMBRE];
        //  strcat(buffer, buffer_entradas[i].nombre);
        sprintf(nombre, "%s", Entradas[i % (BLOCKSIZE / sizeof(struct entrada))].nombre);
        strcat(buffer, nombre);
        strcat(buffer, RESET);
        strcat(buffer, "\n");
        if (offset % (BLOCKSIZE / sizeof(struct entrada)) == 0)
        {
            offset += mi_read_f(p_inodo, Entradas, offset, BLOCKSIZE);
        }
    }
    return Nentradas;
}
/**
 * @brief busca una entrada para obtener el nº del inodo
 * @param   camino  directorio
 * @param   permisos  permisos del directorio
 * @return  EXITO/FALLO
 *
 */
int mi_chmod(const char *camino, unsigned char permisos)
{
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int err = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos);
    if (err < 0)
    {
        return err;
    }

    // si la entrada existe llamamos a la siguietne funcion
    if (mi_chmod_f(p_inodo, permisos) == FALLO)
    {
        fprintf(stderr, RED "mi_chmod: Error en mi_chmod_f\n" RESET);
        return FALLO;
    }
    return EXITO;
}

/**Crea un fichero/directorio y su entrada de directorio.
 * @param   camino  directorio
 * @param   permisos  permisos del directorio
 * @return  EXITO/FALLO
 */

int mi_creat(const char *camino, unsigned char permisos)
{
    //comprobamos que los permisos son válidos
    if (permisos>7 || permisos<0){
        fprintf(stderr, RED "mi_creat: Permisos no válidos\n"RESET);
        return FALLO;
    }
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    return EXITO;
}

/**
 * @brief Busca la entrada camino para obtener el p_inodo
 * @param camino directorio
 * @param p_stat estructura estado
 * @return  p_inodo buscado o FALLO.
 */

int mi_stat(const char *camino, struct STAT *p_stat)
{
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, p_stat->permisos);
    if (error < 0)
    {
        return error;
    }

    // si la entrada existe

    if (mi_stat_f(p_inodo, p_stat) == FALLO)
    {
        return FALLO;
    }

    return p_inodo;
}

/**
 *
 * @brief Escribe el contenido de un buffer en un fichero especificado por
 * su ruta.
 * @return bytes escritos
 */

int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes){

    // primero buscaremos la entrada

    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;

    struct UltimaEntrada UltimaEntradaEscritura;
    memset(&UltimaEntradaEscritura, 0, sizeof(UltimaEntradaEscritura));

    //int bytes_escritos = 0;
    // Sin caché
    // if (USARCACHE == 0)
    // {
        // byscamos primero el ninodo
        // int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        // if (error < 0)
        // {
        //     return error;
        // }
        //bytes_escritos = mi_write_f(p_inodo, buf, offset, nbytes);
        //se asigna el p_inodo con buscar_entrada         
//     }
//     else if (USARCACHE == 1) { // Ultima Entrada del Lectura

        if (strcmp(UltimaEntradaEscritura.camino, camino) == 0)
        {
            p_inodo = UltimaEntradaEscritura.p_inodo;
        }
        else
        {
            int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
            if (error < 0) {
                return error;
            }
            //actualizamos la ultima entrada
            strcpy(UltimaEntradaEscritura.camino, camino);
            UltimaEntradaEscritura.p_inodo = p_inodo;
            #if DEBUGN9
            fprintf(stderr, YELLOW "[mi_write() → Actualizamos la caché de escritura])\n"RESET);
            #endif
        }
//         }
//     }else{

//  // buscamos si la entrada existe
//         int found = 1; // false
//         int index = 0;
//         while (index < CACHE_SIZE && found == 1)
//         {
//             if (strcmp(UltimasEntradas[index].camino, camino) == 0)
//             {
//                 found = 0; // true
//             }
//             index++;
//         }

//         // hay tres escenarios
//         // CASO 1 --> La entrada existe en la tabla
  
//         if (found == 0) {
//             p_inodo = UltimasEntradas[index].p_inodo;
//            // bytes_escritos = mi_write_f(UltimasEntradas[index].p_inodo, buf, offset, nbytes);
//         }

//         //La entrada no existe y hay que buscar si tenemos hueco en la tabla 
//         else if (found == 1) {

//             // buscamos si hay hueco
//             int hueco = -1;
//             int index = 0;
//             while (index < CACHE_SIZE && UltimasEntradas[index].camino != NULL)
//             {
//                 index++;
//             }
//             if (index < CACHE_SIZE)
//             {
//                 hueco = index;
//             }
//             //buscaremos la entrada y actualizaremos el p_inodo
//             int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
//             if (error < 0)
//             {
//                 return error;
//             }


//      if (USARCACHE == 2) { // TABLA FIFO
      
//             // CASO 2 --> La entrada no existe y la podemos añadir
//             if (hueco != -1)
//             {
//                 strcpy(UltimasEntradas[hueco].camino, camino);
//                 UltimasEntradas[hueco].p_inodo = p_inodo;
//                 ultima_insercion = hueco;
//             }
//             else
//             {
//                 // CASO 3 --> La entrada no existe y no queda espacio para añadirla--> criterio FIFO
//                  int pos = (ultima_insercion + 1) % CACHE_SIZE;
//                 strcpy(UltimasEntradas[pos].camino, camino);
//                 UltimasEntradas[pos].p_inodo = p_inodo;
//                 ultima_insercion = pos;
//             }
//         }
    
//      if (USARCACHE == 3) { // LRU
//       struct timeval momento_actual;
//     if(gettimeofday(&momento_actual, NULL) < 0){
//         return FALLO;
//     } 

//     long fechaAntigua = momento_actual.tv_sec;
//  // CASO 2 --> La entrada no existe y la podemos añadir
//             if (hueco != -1)
//             {
//                 strcpy(UltimasEntradas[hueco].camino, camino);
//                 UltimasEntradas[hueco].p_inodo = p_inodo;
//                 UltimasEntradas[hueco].ultima_consulta= momento_actual;
//             } else{
//              // CASO 3 --> La entrada no existe y no queda espacio para añadirla--> criterio FIFO
//                 int pos = -1;
//                 //buscamos si hay alguna entrada con fecha más antigua
//                 for (int i =0; i<CACHE_SIZE; i++){
//                 if (UltimasEntradas[i].camino!=NULL && UltimasEntradas[i].ultima_consulta.tv_sec<fechaAntigua){
//                          fechaAntigua= UltimasEntradas[i].ultima_consulta.tv_sec;
//                          pos=i;   

//                 }
//                 }

//                 strcpy(UltimasEntradas[pos].camino, camino);
//                 UltimasEntradas[pos].p_inodo = p_inodo;
//                 UltimasEntradas->ultima_consulta= momento_actual;
//             }

//         }

//         // si existe la leemos directamente

//     }
// }
    return mi_write_f(p_inodo, buf, offset, nbytes);

}


/**
 * @brief Función de directorios.c para leer los nbytes del fichero
 *  indicado por camino, a partir del offset pasado por parámetro y copiarlos en el buffer buf. 
 * @return los bytes escritos
*/

int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes){

struct UltimaEntrada UltimaEntradaLectura;
memset(&UltimaEntradaLectura, 0, sizeof(UltimaEntradaLectura));
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    if (strcmp(UltimaEntradaLectura.camino, camino) == 0)
        {
            p_inodo = UltimaEntradaLectura.p_inodo;
        }
        else
        {
            int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
            if (error < 0) {
                return error;
            }
            //actualizamos la ultima entrada
            strcpy(UltimaEntradaLectura.camino, camino);
            UltimaEntradaLectura.p_inodo = p_inodo;
            #if DEBUGN9
            fprintf(stderr, YELLOW "Actualizamos la caché mi read()\n"RESET);
            #endif
        }
  
   return mi_read_f(p_inodo,buf,offset,nbytes);
}

/**
 * @brief Función que borra la entrada de directorio especificada. Si es el último
 * enlace existente se borrará el propio fichero/directorio.
 * 
*/
int mi_unlink(const char *camino){

    //comprobamos que la ruta existe
    unsigned int p_inodo_dir=0, p_inodo=0, p_entrada=0;
    //modo consulta y con permisos de lectura
    int error = buscar_entrada(camino,&p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
    if (error<0){
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
  
   struct inodo inodo;
       
       if (leer_inodo(p_inodo, &inodo)<0){
        fprintf(stderr, RED "mi_unlink: Error al leer el inodo de %s\n"RESET, camino);
        return FALLO;
       }

    //si la entrada es de un directorio
    if (camino[strlen(camino)-1]=='/'){
        //y no está vacío
       if (inodo.tamEnBytesLog>0 && inodo.tipo=='d'){
        fprintf(stderr, RED "Error: El directorio %s no está vacío \n"RESET, camino);
        return FALLO;
       }
    }

   
        struct inodo inodo_dir;
        if (leer_inodo(p_inodo_dir, &inodo_dir)==FALLO){
       fprintf(stderr, RED "mi_unlink: Error al leer el inodo de entrada \n"RESET);
       return FALLO;
        }
        unsigned int numEntradas = inodo_dir.tamEnBytesLog/sizeof(struct entrada);

       //Si no es la última entrada
        if (p_entrada!=numEntradas-1){
           
        //leemos la ultima entrada y la colocamos en la posición de la entrada a eliminar
       struct entrada entrada;
       //obtenemos directamente la ultima entrada del directorio
       if (mi_read_f(p_inodo_dir, &entrada,(numEntradas-1)*sizeof(struct entrada),sizeof(struct entrada))<0){
        perror("Error");
        return FALLO;
       }
       if (mi_write_f(p_inodo_dir, &entrada, p_entrada*sizeof(struct entrada), sizeof(struct entrada))<FALLO){
        perror("Error");
        return FALLO;
       }
        }
    //truncar el inodo
         if (mi_truncar_f(p_inodo_dir, (numEntradas-1) * sizeof(struct entrada)) == FALLO) {
        return FALLO;
    }

    // //leemos el inodo asociado al entrada eliminada
    //     if (leer_inodo(p_inodo_dir, &inodo)==FALLO){
    //         return FALLO;
    //     }
        //decrementamos la cantidad de enlaces
       inodo.nlinks--;
        if (inodo.nlinks==0){
            //liberamos el inodo
            if (liberar_inodo(p_inodo)==FALLO){
                fprintf(stderr, RED "mi_unlink: Error al liberar el inodo del directorio %s\n"RESET, camino);
                return FALLO;
            }
        }else{
            //actualizamos el tiempo de actualizacion del inodo
            inodo.ctime= time(NULL);
            if (escribir_inodo(p_inodo, &inodo)==FALLO){
                return FALLO;
            }
        }

        
    
    return EXITO;

}


/**
 * Crea el enlace de una entrada de directorio camino2 al inodo
 * especificado por otra entrada de directorio camino1.
 * 
*/


int mi_link(const char *camino1, const char *camino2){

// Comprobamos que ambos caminos corresponden a ficheros
if ((camino1[strlen(camino1)-1]=='/')||(camino2[strlen(camino2)-1]=='/') ){

    fprintf(stderr, RED "mi_link: Las rutas deben corresponder a ficheros\n"RESET);
    return FALLO;
}
//comprobamos que la entrada camino1 exsita
unsigned int p_inodo_dir1=0, p_inodo1=0, p_entrada1=0;
int error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 4);
if (error<0){
    mostrar_error_buscar_entrada(error);
    return FALLO;
}

struct inodo inodo1;
if (leer_inodo(p_inodo1, &inodo1)==FALLO){
    perror("Error");
    return FALLO;
}

//tenemos permisos de lectura?
if ((inodo1.permisos & 4)!= 4){
    fprintf(stderr, RED "mi_link: No tenemos permisos de lectura\n "RESET);
    return FALLO;
}

//comprobamos que la segunda entrada no exista
unsigned int p_inodo_dir2=0, p_inodo2=0, p_entrada2=0;
 error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6);
if (error<0){
    mostrar_error_buscar_entrada(error);
    return FALLO;
}

//leemos la entrada del camino 2
struct entrada entrada2;

if (mi_read_f(p_inodo_dir2,&entrada2, sizeof(struct entrada)*p_entrada2, sizeof(struct entrada))<0){
    return FALLO;
}

//asociamos a la entrada2 el mismo inodo del inodo1
entrada2.ninodo=p_inodo1;


//Escribimos la entrada modificada en p_inod__dir2
if (mi_write_f(p_inodo_dir2, &entrada2, p_entrada2*sizeof(struct entrada), sizeof(struct entrada))==FALLO){
   fprintf(stderr, RED "Error al escribir la entrada\n"RESET);
    return FALLO;
}
//liberamos el inodo asociado al p_inodo2
if (liberar_inodo(p_inodo2)==FALLO){
    perror("Error");
    return FALLO;
}
inodo1.nlinks++;
inodo1.ctime= time(NULL);
if (escribir_inodo(p_inodo1,&inodo1)==FALLO){
    fprintf(stderr, RED "mi_link: Error al salvar el inodo\n");
    return FALLO;
}
return EXITO;
}

/**
 * 
 * Borra recusrivamente el contenido de un directorio no vacío.
*/

// int mi_rm_r (const char *ruta) {

// unsigned int p_inodo_dir, p_inodo, p_entrada;
// int error = buscar_entrada(ruta, &p_inodo_dir, &p_inodo, &p_entrada, 0, );
// if (error<0){
//     mostrar_error_buscar_entrada(error);
//     return FALLO;
// }

// if (ruta[strlen(ruta)-1]!='/'){//es un fichero
//     if (mi_unlink(ruta)<0){
//         fprintf(stderr, RED "mi_rm_r: Error al borrar el fichero %s\n"RESET, ruta);
//     }
//     return EXITO;
// }
// //Caso de directorio

// struct inodo inodo_dir;
// if (leer_inodo(p_inodo_dir, &inodo_dir)==FALLO){
//     fprintf(stderr, RED"mi_rm_r: Error al leer el inodo del directorio\n"RESET);
//     return FALLO;
// }
// int nEntradas = inodo_dir.tamEnBytesLog/sizeof(struct entrada);
// if (nEntradas==0){
//     fprintf(stderr, RED "mi_rm_r: l directorio es vacío\n"RESET);
//     return FALLO;
// }else{
//     struct entrada Entradas[nEntradas];
//     if (mi_read_f(p_inodo, &Entradas, 0, nEntradas*sizeof(struct entrada)==FALLO)){
//         perror("Error");
//         return FALLO;
//     }
//     for (int i = 0; i<nEntradas; i++){
//         //modificar el nombre de la ruta para poder llamar a la funcion recursiva
//         /*
//         ejemplo : 
//         ruta = /dir/dir1/-->/dir1/ 
//         */
//          char camino_inodo[strlen(ruta) + 1 + strlen(Entradas[i].nombre)];
//         memset(camino_inodo, 0, strlen(ruta) + 1 + strlen(Entradas[i].nombre));
//         strcpy(camino_inodo, ruta);
//         strcat(camino_inodo, Entradas[i].nombre);
//         struct inodo inodo;
//         if (leer_inodo(Entradas[i].ninodo, &inodo) < 0) {
//             return FALLO;
//         }
//         if (inodo.tipo == 'd') {
//             strcat(camino_inodo, "/");
//         }
//         if ((error = mi_rm_r(camino_inodo)) < 0) {
//             return error;
//         }
    

//     }

// }
//     return EXITO;
// }
