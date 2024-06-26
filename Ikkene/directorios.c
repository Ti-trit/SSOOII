/**
 * @author Khaoula Ikkene
 **/

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
    // Variables
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
    // Es el directorio raíz?
    if (strcmp(camino_parcial, "/") == 0)
    {
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return EXITO;
    }
    // inicializamos con cero las variables
    memset(inicial, 0, sizeof(inicial));
    memset(final, 0, strlen(camino_parcial));
    memset(entrada.nombre, 0, sizeof(entrada.nombre));

    if (extraer_camino(camino_parcial, inicial, final, &tipo) < 0)
    {
        return ERROR_CAMINO_INCORRECTO;
    }

#if DEBUGN8 || DEBUGN7
    fprintf(stderr, GRAY "[buscar_entrada()→ inicial: %s, final: %s, reservar: %i]\n" RESET, inicial, final, reservar);
#endif

    if (leer_inodo(*p_inodo_dir, &inodo_dir) == FALLO)
    {
        fprintf(stderr, RED "buscar_entrada; Error en leer_inodo %ls\n" RESET, p_inodo_dir);
        return FALLO;
    }

    // Tenemos permisos de lectura?
    if ((inodo_dir.permisos & 4) != 4)
    {
        return ERROR_PERMISO_LECTURA;
    }

    // calculamos la cantidad de entradas que contiene el inodo
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    num_entrada_inodo = 0; // nº de entrada inicial
    struct entrada bufferEntradas[BLOCKSIZE / sizeof(struct entrada)];
    // inicializar el bufferEntradas de lectura con 0s
    if (memset(&bufferEntradas, 0, sizeof(bufferEntradas)) == NULL)
    {
        fprintf(stderr, RED "buscar_entrada: Error en el memset()\n" RESET);
        return FALLO;
    }
    int leidos = 0;

    if (cant_entradas_inodo > 0)
    {
        // leemos las entradas en el buffer de entradas
        leidos += mi_read_f(*p_inodo_dir, &bufferEntradas, leidos, BLOCKSIZE);
        if (leidos < 0)

        {
            fprintf(stderr, RED "buscar_entrada: Error al leer el bloque de entrada\n" RESET);
            return FALLO;
        }

        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(bufferEntradas[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))].nombre, inicial) != 0))
        {
            num_entrada_inodo++;
            // leer siguiente entrada
            if ((num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))) == 0)
            { // si numEntradas> tamaño de bufferEntradas
                // inicializar el buffer de lectura con 0s
                memset(bufferEntradas, 0, sizeof(bufferEntradas));
                // siguiente lectura de entradas
                leidos += mi_read_f(*p_inodo_dir, bufferEntradas, leidos, BLOCKSIZE);
            }
        }
        // obtenemos la ultima lectura y la guardamos en la variable entrada
        memcpy(&entrada, &bufferEntradas[num_entrada_inodo % (BLOCKSIZE / sizeof(struct entrada))], sizeof(struct entrada));
    }
    // hasta Aqui
    if ((strcmp(inicial, entrada.nombre) != 0) && num_entrada_inodo == cant_entradas_inodo)
    { // La entrada no existe
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
            if (strncpy(entrada.nombre, inicial, sizeof(entrada.nombre)) == NULL)
            {
                fprintf(stderr, RED "buscar_entrada: Error en strcpy()\n" RESET);
                return FALLO;
            }
            // Caso de directorio
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
                // asignar el inodo reservado a la entrada
                entrada.ninodo = ninodo;
#if DEBUGN8 || DEBUGN7
                fprintf(stderr, GRAY "[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n" RESET, entrada.ninodo, tipo, permisos, entrada.nombre);
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
                entrada.ninodo = ninodo;
#if DEBUGN8 || DEBUGN7
                fprintf(stderr, GRAY "[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n" RESET, entrada.ninodo, tipo, permisos, entrada.nombre);
#endif
            }

#if DEBUGN8 || DEBUGN7

            fprintf(stderr, GRAY "[buscar_entrada()→ creada entrada: %s, %d] \n" RESET, inicial, entrada.ninodo);
#endif
            // escribir la entrada en el directorio padre
            if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(entrada), sizeof(entrada)) == FALLO)
            {
                int ninodo = entrada.ninodo;
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
        // cortamos la recursividad
        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;
        return EXITO;
    }
    else
    { // llamada recursiva
        *p_inodo_dir = entrada.ninodo;
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
        // mostrar_error_buscar_entrada(err);
        // return FALLO;

        return err;
    }

    struct inodo inodo;
    if (leer_inodo(p_inodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "mi_dir: Error al leer el inodo %d" RESET, p_inodo);
        return FALLO;
    }
    if ((inodo.permisos & 4) != 4)
    {
        // fprintf(stderr, RED "No tiene permisos de lectura\n" RESET);
        // return FALLO;
        return ERROR_PERMISO_LECTURA;
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

        struct tm *tm;
        char tmp[80];
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
    // comprobamos que los permisos son válidos
    if (permisos > 7 || permisos < 0)
    {
        fprintf(stderr, RED "mi_creat: Permisos no válidos\n" RESET);
        return FALLO;
    }
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    mi_waitSem();
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
    mi_signalSem();
    if (error < 0)
    {
        // mostrar_error_buscar_entrada(error);
        // return FALLO;
        return error;
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
 * @brief Función que borra la entrada de directorio especificada. Si es el último
 * enlace existente se borrará el propio fichero/directorio.
 *
 */
int mi_unlink(const char *camino)
{

    // comprobamos que la ruta existe
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    mi_waitSem();

    // modo consulta y con permisos de lectura
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return FALLO;
    }

    struct inodo inodo;

    if (leer_inodo(p_inodo, &inodo) < 0)
    {
        fprintf(stderr, RED "mi_unlink: Error al leer el inodo de %s\n" RESET, camino);
        mi_signalSem();
        return FALLO;
    }

    // si la entrada es de un directorio
    if (camino[strlen(camino) - 1] == '/')
    {
        // y no está vacío
        if (inodo.tamEnBytesLog > 0 && inodo.tipo == 'd')
        {
            fprintf(stderr, RED "Error: El directorio %s no está vacío \n" RESET, camino);
            mi_signalSem();
            return FALLO;
        }
    }

    struct inodo inodo_dir;
    if (leer_inodo(p_inodo_dir, &inodo_dir) == FALLO)
    {
        fprintf(stderr, RED "mi_unlink: Error al leer el inodo de entrada \n" RESET);
        mi_signalSem();
        return FALLO;
    }
    unsigned int numEntradas = inodo_dir.tamEnBytesLog / sizeof(struct entrada);

    // Si no es la última entrada
    if (p_entrada != numEntradas - 1)
    {

        // leemos la ultima entrada y la colocamos en la posición de la entrada a eliminar
        struct entrada entrada;
        // obtenemos directamente la ultima entrada del directorio
        if (mi_read_f(p_inodo_dir, &entrada, (numEntradas - 1) * sizeof(struct entrada), sizeof(struct entrada)) < 0)
        {
            perror("Error");
            mi_signalSem();
            return FALLO;
        }
        if (mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) < 0)
        {
            perror("Error");
            mi_signalSem();
            return FALLO;
        }
    }
    // truncar el inodo
    if (mi_truncar_f(p_inodo_dir, (numEntradas - 1) * sizeof(struct entrada)) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }

    // //leemos el inodo asociado al entrada eliminada
    //     if (leer_inodo(p_inodo_dir, &inodo)==FALLO){
    //         return FALLO;
    //     }
    // decrementamos la cantidad de enlaces
    inodo.nlinks--;
    if (inodo.nlinks == 0)
    {
        // liberamos el inodo
        if (liberar_inodo(p_inodo) == FALLO)
        {
            fprintf(stderr, RED "mi_unlink: Error al liberar el inodo del directorio %s\n" RESET, camino);
            //   mi_signalSem();
            return FALLO;
        }
    }
    else
    {
        // actualizamos el tiempo de actualizacion del inodo
        inodo.ctime = time(NULL);
        if (escribir_inodo(p_inodo, &inodo) == FALLO)
        {
            //     mi_signalSem();
            return FALLO;
        }
    }

    mi_signalSem();
    return EXITO;
}

/**
 * Crea el enlace de una entrada de directorio camino2 al inodo
 * especificado por otra entrada de directorio camino1.
 *
 */

int mi_link(const char *camino1, const char *camino2)
{

    unsigned int p_inodo_dir1 = 0, p_inodo1 = 0, p_entrada1 = 0, p_inodo_dir2 = 0, p_inodo2 = 0, p_entrada2 = 0;

    if (!camino1 || !camino2)
    {
        return FALLO;
    }

    mi_waitSem();

    if (camino1[strlen(camino1) - 1] == '/' || camino2[strlen(camino2) - 1] == '/')
    { // Comprobar que las rutas no apunten a ficheros
        mi_signalSem();
        return FALLO;
    }
    int error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 4);
    // mi_signalSem();
    if (error < 0)
    { // Buscar entrada de camino1
        mi_signalSem();
        return error;
    }

    error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6);
    if (error < 0)
    { // Crear entrada para camino2
        mi_signalSem();
        return error;
    }

    struct inodo inodo1;
    if (leer_inodo(p_inodo1, &inodo1) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }

    struct entrada entrada; // Leemos la entrada creada correspondiente a camino2, o sea la entrada p_entrada2 de p_inodo_dir2.
    if (mi_read_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
    {
        mi_signalSem();
        return FALLO;
    }

    entrada.ninodo = p_inodo1; // Creamos el enlace: Asociamos a esta entrada el mismo inodo que el asociado a la entrada de camino1, es decir p_inodo1.

    if (mi_write_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
    { // Escribimos la entrada modificada en p_inodo_dir2.
        mi_signalSem();
        return FALLO;
    }

    if (liberar_inodo(p_inodo2) == FALLO)
    { // Liberamos el inodo que se ha asociado a la entrada creada, p_inodo2.
        mi_signalSem();
        return FALLO;
    }

    inodo1.nlinks++; // Incrementamos la cantidad de enlaces (nlinks) de p_inodo1, actualizamos el ctime y lo salvamos.
    inodo1.ctime = time(NULL);

    if (escribir_inodo(p_inodo1, &inodo1) == FALLO)
    { // guardar el inodo
        //   mi_signalSem();
        return FALLO;
    }
    mi_signalSem();
    return EXITO;
}

#if (USARCACHE == 2 || USARCACHE == 3)
static struct UltimaEntrada UltimasEntradas[CACHE_SIZE];
#endif

#if USARCACHE == 1 // ultima L/E
struct UltimaEntrada UltimaEntradaEscritura;
#endif

/**
 * @brief Función de directorios.c para leer los nbytes del fichero
 *  indicado por camino, a partir del offset pasado por parámetro y copiarlos en el buffer buf.
 * @return los bytes escritos
 */
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
{

    // primero buscaremos la entrada
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;

// Sin caché
#if USARCACHE == 0

    //  byscamos primero el ninodo
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
    if (error < 0)
    {
        return error;
    }

#endif
    // Ultima Entrada del Lectura
#if USARCACHE == 1
    if (strcmp(UltimaEntradaEscritura.camino, camino) == 0)
    {
        p_inodo = UltimaEntradaEscritura.p_inodo;
    }
    else
    {
        // la entrada no existe y la tenemos que buscar
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (error < 0)
        {
            return error;
        }
        // actualizamos la ultima entrada
        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
#if DEBUGN9
        fprintf(stderr, YELLOW "\n[mi_read() → Actualizamos la caché de lectura])\n" RESET);
#endif
    }
#endif

#if (USARCACHE == 2)
    // La entrada esta en la tabla?
    int index = 0;
    int found = 0;
    while (index < CACHE_SIZE && found == 0)
    {
        if ((UltimasEntradas[index].camino != NULL && strcmp(UltimasEntradas[index].camino, camino) == 0))
        {
            found = 1;
            p_inodo = UltimasEntradas[index].p_inodo;
#if DEBUGN9
            fprintf(stderr, YELLOW "\n[mi_read() → Utilizamos la caché de lectura en vez de llamar a buscar_entrada()]\n" RESET);
#endif

#if DEBUGN9_PREUBA
            fprintf(stderr, YELLOW "\n[mi_read() → Utilizamos cache[%i] : %s]\n" RESET, index, camino);
#endif

            break;
        }
        index++;
    }

    if (found == 0) // no existe en la tabla
    {
        // obtenemos el p_inodo de la entrada
        int er = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (er < 0)
        { // modo consulta y permisos de lectura
            return er;
        }

        int pos = (UltimasEntradas->ultima_insercion) % CACHE_SIZE; // buscamos la posicion para hacer la insercion
        strcpy(UltimasEntradas[pos].camino, camino);                // copiamos el camino

        UltimasEntradas[pos].p_inodo = p_inodo;
        UltimasEntradas->ultima_insercion++; // actualizamos el valor de la ultima insercion
        fprintf(stderr, ORANGE "[mi_read() → Reemplazamos cache[%i]: %s ]\n" RESET, pos, camino);

#if DEBUGN9
        // fprintf(stderr, YELLOW "\n[mi_read() → Actualizamos la caché de lectura]\n" RESET);
#endif
    }

#endif

#if (USARCACHE == 3) // LRU

    
    // La entrada esta ya en la tabla?
struct timeval momento_actual;
    
    int index = 0;
    int found = 0;
    while (index < CACHE_SIZE && found == 0)
    {
        if ((UltimasEntradas[index].camino != NULL && strcmp(UltimasEntradas[index].camino, camino) == 0))
        {
            found = 1;
            p_inodo = UltimasEntradas[index].p_inodo;

#if DEBUGN9_PREUBA
            fprintf(stderr, BLUE "[mi_read()→ Utilizamos cache[%i]: %s]\n" RESET, index, camino);
#endif

#if DEBUGN9
            fprintf(stderr, YELLOW "\n[mi_read() → Utilizamos la caché de lectura en vez de llamar a buscar_entrada()]\n" RESET);
#endif
             if (gettimeofday(&momento_actual, NULL) < 0)
                {
                 return FALLO;
                 }
            UltimasEntradas[index].ultima_consulta = momento_actual; // actualizamos el tiempo de referencia
            break;
        }
        index++;
    }

    if (found == 0)
    { // si no existe en la tabla
        if (gettimeofday(&momento_actual, NULL) < 0)
            {
            return FALLO;
    }
        // buscamos la entrada para obtener su n1
        int er = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (er < 0)
        { // Obtener p_inodo de la entrada
            return er;
        }
#if DEBUGN9
        //  fprintf(stderr, YELLOW "\n[mi_read() → Actualizamos la caché de lectura]\n"RESET);
#endif
        if (index < CACHE_SIZE) // lA ENTRADA no existe y la podemos añadir
        {

            strcpy(UltimasEntradas[index].camino, camino);
            UltimasEntradas[index].p_inodo = p_inodo;
             
            UltimasEntradas[index].ultima_consulta = momento_actual;
        }
        else
        {
            // no existe y no hay espacio--> sustituye a la que hace mas tiempo sin ser referenciada

            struct timeval fechaAntigua = momento_actual;

            int pos = -1;
            // buscamos si hay alguna entrada con fecha más antigua
            for (int i = 0; i < CACHE_SIZE; i++)
            {
                if (UltimasEntradas[i].camino != NULL && UltimasEntradas[i].ultima_consulta.tv_usec < fechaAntigua.tv_usec)
                {
                    fechaAntigua = UltimasEntradas[i].ultima_consulta;
                    pos = i;
                }
            }
#if DEBUGN9_PREUBA
            fprintf(stderr, ORANGE "[mi_read() → Reemplazamos cache[%i]: %s]\n" RESET, pos, camino);
#endif

            strcpy(UltimasEntradas[pos].camino, camino);
            UltimasEntradas[pos].p_inodo = p_inodo;
            UltimasEntradas[pos].ultima_consulta = momento_actual;
        }
    }
#endif

    return mi_read_f(p_inodo, buf, offset, nbytes);
}

/**
 *
 * @brief Escribe el contenido de un buffer en un fichero especificado por
 * su ruta.
 * @return bytes escritos
 */

int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{

    // primero buscaremos la entrada
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;

// Sin caché
#if USARCACHE == 0

    //  byscamos primero el ninodo
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
    if (error < 0)
    {
        return error;
    }

#endif
    // Ultima Entrada del Lectura
#if USARCACHE == 1
    if (strcmp(UltimaEntradaEscritura.camino, camino) == 0)
    {
        p_inodo = UltimaEntradaEscritura.p_inodo;
    }
    else
    {
        // la entrada no existe y la tenemos que buscar
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (error < 0)
        {
            return error;
        }
        // actualizamos la ultima entrada
        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
#if DEBUGN9
        fprintf(stderr, YELLOW "\n[mi_write() → Actualizamos la caché de escritura])\n" RESET);
#endif
    }
#endif

#if (USARCACHE == 2)
    // La entrada esta en la tabla?
    int index = 0;
    int found = 0;
    while (index < CACHE_SIZE && found == 0)
    {
        if ((UltimasEntradas[index].camino != NULL && strcmp(UltimasEntradas[index].camino, camino) == 0))
        {
            found = 1;
            p_inodo = UltimasEntradas[index].p_inodo;
#if DEBUGN9
            fprintf(stderr, YELLOW "\n[mi_write() → Utilizamos la caché de escritura en vez de llamar a buscar_entrada()]\n" RESET);
#endif

#if DEBUGN9_PREUBA
            fprintf(stderr, YELLOW "\n[mi_write() → Utilizamos cache[%i] : %s]\n" RESET, index, camino);
#endif

            break;
        }
        index++;
    }

    if (found == 0) // no existe en la tabla
    {
        // obtenemos el p_inodo de la entrada
        int er = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (er < 0)
        { // modo consulta y permisos de lectura
            return er;
        }

        int pos = (UltimasEntradas->ultima_insercion) % CACHE_SIZE; // buscamos la posicion para hacer la insercion
        strcpy(UltimasEntradas[pos].camino, camino);                // copiamos el camino

        UltimasEntradas[pos].p_inodo = p_inodo;
        UltimasEntradas->ultima_insercion++; // actualizamos el valor de la ultima insercion
        fprintf(stderr, ORANGE "[mi_write() → Reemplazamos cache[%i]: %s]\n" RESET, pos, camino);

#if DEBUGN9
        // fprintf(stderr, YELLOW "\n[mi_write() → Actualizamos la caché de escritura]\n" RESET);
#endif
    }

#endif

#if (USARCACHE == 3) // LRU

    struct timeval momento_actual;
    if (gettimeofday(&momento_actual, NULL) < 0)
    {
        return FALLO;
    }
    // La entrada esta ya en la tabla?

    int index = 0;
    int found = 0;
    while (index < CACHE_SIZE && found == 0)
    {
        if ((UltimasEntradas[index].camino != NULL && strcmp(UltimasEntradas[index].camino, camino) == 0))
        {
            found = 1;
            p_inodo = UltimasEntradas[index].p_inodo;

#if DEBUGN9_PREUBA
            fprintf(stderr, BLUE "[mi_write()→ Utilizamos cache[%i]: %s]\n" RESET, index, camino);
#endif

#if DEBUGN9
            fprintf(stderr, YELLOW "\n[mi_write() → Utilizamos la caché de escritura en vez de llamar a buscar_entrada()]\n" RESET);
#endif
            UltimasEntradas[index].ultima_consulta = momento_actual; // actualizamos el tiempo de referencia
            break;
        }
        index++;
    }

    if (found == 0)
    { // si no existe en la tabla

        // buscamos la entrada para obtener su n1
        int er = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
        if (er < 0)
        { // Obtener p_inodo de la entrada
            return er;
        }
#if DEBUGN9
        //  fprintf(stderr, YELLOW "\n[mi_write() → Actualizamos la caché de escritura]\n"RESET);
#endif
        if (index < CACHE_SIZE) // lA ENTRADA no existe y la podemos añadir
        {

            strcpy(UltimasEntradas[index].camino, camino);
            UltimasEntradas[index].p_inodo = p_inodo;
            UltimasEntradas[index].ultima_consulta = momento_actual;
        }
        else
        {
            // no existe y no hay espacio--> sustituye a la que hace mas tiempo sin ser referenciada

            struct timeval fechaAntigua = momento_actual;

            int pos = -1;
            // buscamos si hay alguna entrada con fecha más antigua
            for (int i = 0; i < CACHE_SIZE; i++)
            {
                if (UltimasEntradas[i].camino != NULL && UltimasEntradas[i].ultima_consulta.tv_usec < fechaAntigua.tv_usec)
                {
                    fechaAntigua = UltimasEntradas[i].ultima_consulta;
                    pos = i;
                }
            }
#if DEBUGN9_PREUBA
            fprintf(stderr, ORANGE "[mi_write() → Reemplazamos cache[%i]: %s]\n" RESET, pos, camino);
#endif

            strcpy(UltimasEntradas[pos].camino, camino);
            UltimasEntradas[pos].p_inodo = p_inodo;
            UltimasEntradas[pos].ultima_consulta = momento_actual;
        }
    }
#endif

    return mi_write_f(p_inodo, buf, offset, nbytes);
}

// Mejoras extra

/**
 *
 * Borra recusrivamente el contenido de un directorio no vacío.
 */
int mi_rm_r(const char *ruta)
{

    if (ruta[strlen(ruta) - 1] != '/')
    { // es un fichero, llamamos directamente a mi_unlink
        int error = mi_unlink(ruta);
        if (error < 0)
        {
            return error;
        }
        return EXITO;
    }

    // Caso de directorio. Hay que leer sus entradas y borrarlas recursivamente
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int error = buscar_entrada(ruta, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0); // permisos temporales
    if (error < 0)
    {
        //  mostrar_error_buscar_entrada(error);
        return error;
    }

    // Leemos el inodo del directorio para obtener el nº de entradas que tiene
    struct inodo inodo_dir;
    if (leer_inodo(p_inodo, &inodo_dir) == FALLO)
    {
        fprintf(stderr, RED "mi_rm_r: Error al leer el inodo del directorio\n" RESET);
        return FALLO;
    }

    int nEntradas = inodo_dir.tamEnBytesLog / sizeof(struct entrada);

    struct entrada Entradas[nEntradas]; // buffer de entradas
    if (mi_read_f(p_inodo, &Entradas, 0, nEntradas * sizeof(struct entrada)) == FALLO)
    {
        perror("Error");
        return FALLO;
    }

    for (int i = 0; i < nEntradas; i++)
    {
        // modificar el nombre de la ruta para poder llamar a la funcion recursiva
        /*
        ejemplo :
        ruta = /dir/dir1/-->  y la entrada es fic1 -->rutaInodo = /dir/dir1/fic1
        */

        char *nuevoCamino = malloc((TAMNOMBRE*PROFUNDIDAD) + 2); // 1 del caracter / y otro del caracter nulo
        // poner a cero
        memset(nuevoCamino, 0, (TAMNOMBRE*PROFUNDIDAD) + 2);
        strcpy(nuevoCamino, ruta);
        // concatenar el nombre de la nueva ruta a la ruta original
        strcat(nuevoCamino, Entradas[i].nombre);

        // Leemos el inodo de la entrada para determinar su tipo
        struct inodo inodoEntrada;
        if (leer_inodo(Entradas[i].ninodo, &inodoEntrada) < 0)
        {
            return FALLO;
        }
        if (inodoEntrada.tipo == 'd') {
            strcat(nuevoCamino, "/");
        }

        int error = mi_rm_r(nuevoCamino); // llamada recursivaa
        if (error < 0)
        {
            return error;
        }
    }
//borramos el directorio que ahora se encunetra vacío
    error = mi_unlink(ruta);
    return error < 0 ? error : EXITO;
}

/**
 * Renombra un fichero/directorio dentro de un mismo directorio
 */

int mi_rn(char *rutaAntigua, char *nombreNuevo)
{

    // Existe la entrada que cambiaremos su nombre?
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int error = buscar_entrada(rutaAntigua, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0); // modo consulta
    if (error < 0)
    {
        return error;
    }
    // guardamos el nº del inodo y de la entrada
    unsigned int p_inodo_dirInicial = p_inodo_dir;
    unsigned int p_entradaInicial = p_entrada;

    // obtenemos la ruta nueva
    char *puntero_ultima = strrchr(rutaAntigua, '/'); // obtener ultima instancia de /
    int n = strlen(rutaAntigua) - strlen(puntero_ultima);//n es el numero de caracters a copiar de ruta
    char *ruta = malloc(n + strlen(nombreNuevo));
    strncpy(ruta, rutaAntigua, n + 1);
    strcat(ruta, nombreNuevo);

    //antiguo y nuevo han de ser del mismo tipo
    if (rutaAntigua[strlen(rutaAntigua) - 1] == '/') { // es un directorio
        strcat(ruta, "/");
    }

    // comprobamos que la nueva ruta no existe
    p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    error = buscar_entrada(ruta, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (error !=ERROR_NO_EXISTE_ENTRADA_CONSULTA )
    {
        // si la entrada que queremos crear ya existe
        fprintf(stderr, RED "Error: La entrada ya existe\n"RESET);
        return FALLO;
    }

    // leemos la entrada correspondiente
    unsigned long sizeofEntrada = sizeof(struct entrada);
    struct entrada entrada;
    if (mi_read_f(p_inodo_dirInicial, &entrada, p_entradaInicial * sizeofEntrada, sizeofEntrada) < 0)
    {
        fprintf(stderr, RED "mi_rn: Error al leer la entrada\n" RESET);
        return FALLO;
    }
    // actualizamos el nombre y guardamos los cambios
    strcpy(entrada.nombre, nombreNuevo);
    // no hay cambios a nivel del inodo

    if (mi_write_f(p_inodo_dirInicial, &entrada, p_entradaInicial * sizeofEntrada, sizeofEntrada) < 0)
    {
        fprintf(stderr, RED "mi_rn: Error al cambiar el nombre de la entrada\n" RESET);
        return FALLO;
    }

    return EXITO;
}

/**
 * Copia un fichero en otro directorio
 */

int mi_cp_f(char *camino, char *destino)
{

    // comprobamos que camino es la ruta de un fichero
    if (camino[strlen(camino) - 1] == '/')
    {
        fprintf(stderr, RED "La ruta de no es de un fichero %s\n " RESET, camino);
        return FALLO;
    }

    // Comprobamos que camino exista
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (error < 0)
    {
        return error;
    }
    // y destino también
    unsigned int p_id2 = 0, p_i2 = 0, p_entr2 = 0;
    error = buscar_entrada(destino, &p_id2, &p_i2, &p_entr2, 0, 0);
    if (error < 0)
    {
        return error;
    }

    //el destino es un directorio?
    if (destino[strlen(destino)-1]!='/'){
        fprintf(stderr, RED "Error: El destino tiene que ser un directorio\n" RESET);
        return FALLO;

    }

    // creamos un nuevo fichero en destino
    // el camino del fichero es /destino/+ nombre fichero

    char *start_puntero = strrchr(camino, '/'); // obtener ultima instancia de /
    char *nombre = malloc(strlen(start_puntero) + 1);//no copiamos el / porque el destino ya es un directorio
    strcpy(nombre, start_puntero + 1);

    char *rutaNueva = malloc(strlen(camino) + strlen(nombre) + 1); // 1 del caracter nulo
    strcpy(rutaNueva, destino);
    strcat(rutaNueva, nombre);

    //obtenemos el inodo del fichero original
    struct inodo inodo_fichero;
    if (leer_inodo(p_inodo, &inodo_fichero) < 0)
    {
        fprintf(stderr, RED "mi_cp_f: Error al leer la entrada del fichero\n" RESET);
        return FALLO;
    }

    //creamos la entrada del fichero en destino
    p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    //modo reservar y con los mismos persmisos que el ficheor inicial
    int error2 = buscar_entrada(rutaNueva, &p_inodo_dir, &p_inodo, &p_entrada, 1, inodo_fichero.permisos);
    if (error2 < 0)
    {
        fprintf(stderr, RED "mi_cp_f: Error al crear el nuevo fichero\n" RESET);
        return error2;
    }

    // leemos el contenido del fichero y lo volcamos en el nuevo fichero

    unsigned char bufferLectura[BLOCKSIZE], bufferInicializadoCeros[BLOCKSIZE];
    memset(bufferLectura, 0, BLOCKSIZE);
    memset(bufferInicializadoCeros, 0, BLOCKSIZE);
    int offset = 0;
    int leidos_aux = mi_read(camino, bufferLectura, offset, BLOCKSIZE);
    while (leidos_aux > 0){
        if (memcmp(bufferLectura, bufferInicializadoCeros, leidos_aux) != 0)
        { // para no copiar basura
            if (mi_write(rutaNueva, bufferLectura, offset, leidos_aux) < 0)
            {
                fprintf(stderr, RED "mi_cp_f: Error al copiar el contenido\n" RESET);
                return FALLO;
            }
        }

        memset(bufferLectura, 0, BLOCKSIZE);                            // para las nuevas lecturas
        offset += BLOCKSIZE;                                            // actualizamos el offset
        leidos_aux = mi_read(camino, bufferLectura, offset, BLOCKSIZE); // leemos de nuevo

    }

    if (leidos_aux < 0)
    {
        return leidos_aux;
    }

    //liberar memoria asignada
    free(nombre);
    free(rutaNueva);

    return EXITO;
}

/**
 * Copia un fichero o un directorio a otro directorio
 */

int mi_cp(char *camino, char *destino) {

    if (destino[strlen(destino) - 1] != '/') {
        fprintf(stderr, RED "Error: El destino tiene que ser un directorio\n" RESET);
        return FALLO;
    }

    // comprobamos que destino existe
    unsigned int p_inododir = 0, pinodo = 0, pentrada = 0;
    int erreur = buscar_entrada(destino, &p_inododir, &pinodo, &pentrada, 0, 0);
    if (erreur < 0)
    {
        return erreur;
    }

    // si es un fichero llamar directamente a mi_cp_f
    if (camino[strlen(camino) - 1] != '/')
    {
        erreur = mi_cp_f(camino, destino);
        if (erreur < 0)
        {
            return erreur;
        }
    }
    else
    { // caso de directorios
        unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
        // leemos la entrada del directorio para obtener info de su inodo
        int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
        unsigned int p_inodoInicial = p_inodo;
        if (error < 0)
        {
            return error;
        }
       

        // creamos un entrada para el directorio en destino
        int size = strlen(camino);
        char *camino_auxiliar = (char *)malloc(size);
        memset(camino_auxiliar, 0, size);
        strncpy(camino_auxiliar, camino, size - 1); // para no coger el ultimo /
        camino_auxiliar[size - 1] = '\0';           // fin de cadena
        //obtenemos el nombre
        char *start_puntero = strrchr(camino_auxiliar, '/'); // obtener ultima instancia de /
        char *nombre = malloc(strlen(start_puntero) + 1);
        memset(nombre, 0, strlen(start_puntero) + 1);
        strcpy(nombre, start_puntero + 1);
    //Obtenemos la nueva ruta 
        char *rutaNueva = malloc(strlen(destino) + strlen(nombre) + 2); // 1 del caracter nulo + 1 del caracter /
        memset(rutaNueva, 0, strlen(destino) + strlen(nombre) + 2);
        strcpy(rutaNueva, destino);
        strcat(rutaNueva, nombre);
        strcat(rutaNueva, "/"); // ya que es un directorio

            //obtenemos los permisos del inodo del directorio actual
         struct inodo inodo_actual;
        if (leer_inodo(p_inodo, &inodo_actual) < 0)
        {
            fprintf(stderr, RED "mi_cp: Error al leer el inodo del directorio \n" RESET);
            return FALLO;
        }
        
        // reiniciar las variables
        p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
        // creamos la entrada en destino con los mismos permisos
        error = buscar_entrada(rutaNueva, &p_inodo_dir, &p_inodo, &p_entrada, 1, inodo_actual.permisos);
        if (error < 0)
        {
            return error;
        }
        // obtenemos numero de entradas
        int nEntradas = inodo_actual.tamEnBytesLog / sizeof(struct entrada);
        struct entrada Entradas[nEntradas]; // buffer de entradas
        //leemos las entradas del directorio y las guardamos en el buffer
        if (mi_read_f(p_inodoInicial, &Entradas, 0, nEntradas * sizeof(struct entrada)) < 0)
        {
            perror("Error");
            return FALLO;
        }

        // para cada entrada del directorio actual llamaremos a mi_cp
        // si es un fichero se llamara a mi_cp_f
        // si no, se leen las entradas del directorio actual
        // y se repite el mismo proceso

            int i = 0; 
            while (i<nEntradas){
            char *nuevoCamino = malloc((TAMNOMBRE*PROFUNDIDAD)+2);
            memset(nuevoCamino, 0, (TAMNOMBRE*PROFUNDIDAD)+2);
            // poner a cero
            if (memset(nuevoCamino, 0, (TAMNOMBRE*PROFUNDIDAD)+2) == NULL)
            {
                fprintf(stderr, RED "mi_cp: Error al asignar memoria al nuevo camino\n" RESET);
                return FALLO;
            }
            // obtenemos la ruta de la entrada actual
            strcpy(nuevoCamino, camino);
            strcat(nuevoCamino, Entradas[i].nombre);

            // Leemos el inodo de la entrada para determinar su tipo
            struct inodo inodoEntrada;
            if (leer_inodo(Entradas[i].ninodo, &inodoEntrada) < 0)
            {
                fprintf(stderr, RED "mi_cp: Error el leer el inodo%i\n"RESET, Entradas[i].ninodo);
                return FALLO;
            }
            if (inodoEntrada.tipo == 'd')
            {
                // es un directorio
                strcat(nuevoCamino, "/");
            }
            i++;

            int err = mi_cp(nuevoCamino, rutaNueva);
            if (err < 0)
            {
                return err;
            }
        }

        //liberamos memoria
        free(nombre);
        free(camino_auxiliar);
        free(rutaNueva);
    }

    return EXITO;
}

/**
 * Función que mueve un fichero/directorio a otro directorio
 *@return EXITO si ha ido bien el proceso. 
        FALLO o error en caso contrario 
 */

int mi_mv(char *camino, char *destino){

        // existe la entrada del camino?
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);
    if (error < 0)
    {
        return error;
    }
    // y la del destino?
    unsigned int destino_inodo_dir=0, destino_inodo=0, destino_entrada=0;
    error = buscar_entrada(destino, &destino_inodo_dir, &destino_inodo, &destino_entrada, 0, 0);
    if (error<0){
        return error;
    }
    // destino tiene que ser un directorio
    if (destino[strlen(destino)-1]!='/'){
         fprintf(stderr, RED "Error: El destino tiene que ser un directorio\n" RESET);
        return FALLO;
    }

    long sizeEntrada = sizeof(struct entrada);
    struct entrada entradaCamino;
    if (mi_read_f(p_inodo_dir, &entradaCamino, p_entrada * sizeEntrada, sizeEntrada) == FALLO)
    {
        return FALLO;
    }

    // creamos la ruta de la nueva entrada que movemos
    char *rutaNueva = malloc(strlen(destino) + strlen(entradaCamino.nombre) + 2);// 2 = 1 (caracter nulo)+ 1(/)
    memset(rutaNueva, 0, strlen(destino) + strlen(entradaCamino.nombre) + 2);
    strcpy(rutaNueva, destino);
    strcat(rutaNueva, entradaCamino.nombre);
   
    // si es un directorio
    if (camino[strlen(camino) - 1] == '/')
    {
        strcat(rutaNueva, "/");
    }

    // no debe existir la entrada que hemos creado
    unsigned int p_inodo_dir_destino = 0, p_inodo_destino = 0, p_entrada_destino = 0;

    if (buscar_entrada(rutaNueva, &p_inodo_dir_destino, &p_inodo_destino, &p_entrada_destino, 0, 0) != ERROR_NO_EXISTE_ENTRADA_CONSULTA)
    {
         fprintf(stderr, RED "Error: La entrada ya existe\n"RESET);
        return FALLO;
    }
    // obtenemos los inodos de ambas rutas
    
    struct inodo inodoDestino;
    if (leer_inodo(p_inodo_dir_destino, &inodoDestino) == FALLO)
    {
        fprintf(stderr, RED "mi_mv: Error al obtener el inodo de /destino/\n"RESET);

        return FALLO;
    }

    // creamos la entrada de camino en destino
    if (mi_write_f(p_inodo_dir_destino, &entradaCamino, inodoDestino.tamEnBytesLog, sizeEntrada) == FALLO)
    {
        return FALLO;
    }

    struct inodo inodoCamino;
    if (leer_inodo(p_inodo_dir, &inodoCamino) == FALLO)
    {
        fprintf(stderr, RED "mi_mv: Error al obtener el inodo de /origen/\n"RESET);
        return FALLO;
    }

    unsigned int nEntradas = inodoCamino.tamEnBytesLog / sizeEntrada;
    // borramos la entrada de camino
    
    // Si no es la última entrada
    if (p_entrada != nEntradas - 1)
    {

        // leemos la ultima entrada y la colocamos 
        //en la posición de la entrada a eliminar
        struct entrada entrada;
        // obtenemos directamente la ultima entrada del directorio
        if (mi_read_f(p_inodo_dir, &entrada, (nEntradas - 1) * sizeEntrada, sizeEntrada) < 0)
        {
            return FALLO;
        }
        if (mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeEntrada, sizeEntrada) < 0)
        {

            return FALLO;
        }
    }
    // truncar el inodo
    if (mi_truncar_f(p_inodo_dir, p_entrada * sizeEntrada) == FALLO)
    {
        fprintf(stderr, RED "mi_mv: Error al truncar %s\n"RESET, camino);
        return FALLO;
    }

    free(rutaNueva);

    return EXITO;
}
