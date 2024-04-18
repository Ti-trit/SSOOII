#include "directorios.h"
/**
 * @brief Función que separa una cadena(que empieza por /) en dos partes,  inicial y final.
 * @param   camino    cadena a separar
 * @param   inicial   parte inicial obtenido de camino
 * @param   final     resta de la cadena camino
 * @param   tipo      tipo del camino
 * @return            EXITO o FALLO  
*/
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {
    if (camino[0] != '/') {
        fprintf(stderr, RED"extraer_camino:Error, el camino no contiene / al inicio\n" RESET);
        return FALLO;
    }

    const char *segundo_slash = strchr(camino + 1, '/');
    if (segundo_slash != NULL) {
        // Extraer la posicion hasta el segundo '/'
        int len = segundo_slash - (camino + 1);

        if (strncpy(inicial, camino + 1, len)==NULL){
            perror("extraer_camino:Error strncpy()");
            return FALLO;
        }
        inicial[len] = '\0';

        //Resto del camino
        strcpy(final, segundo_slash);
        *tipo = 'd'; //Es un directorio porque hay un segundo '/'
    } else {
        //No hay segunco '/', por lo tanto es un fichero
        if (strcpy(inicial, camino + 1)==NULL){
            perror("extraer_camino:Error strncpy()");
            return FALLO;
        }
        
        final[0] = '\0';
        *tipo = 'f';
    }

    return EXITO;
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
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;
    struct superbloque SB; 
    struct entrada buffer[BLOCKSIZE/sizeof(struct entrada)];

    //leemos el superbloque
    if (bread(posSB, &SB)==FALLO){
        fprintf(stderr, RED"buscar_entrada: Error en bread()\n"RESET);
        return FALLO;
    }


if (camino_parcial[0]=='/'){    //directorio raiz
    *p_inodo=SB.posInodoRaiz;
    *p_entrada= 0;
    return EXITO;

}

   memset(final, 0, strlen(camino_parcial));
    memset(inicial, 0, sizeof(buffer[0].nombre));

if (extraer_camino(camino_parcial, inicial, final, &tipo)==FALLO){
    mostrar_error_buscar_entrada(ERROR_CAMINO_INCORRECTO);
    return ERROR_CAMINO_INCORRECTO;
}

   //buscamos la entrada cuyo nombre se encuentra en inicial
   if (leer_inodo(*p_inodo_dir, &inodo_dir)==FALLO){
        fprintf(stderr, RED "buscar_entrada; Error en leer_inodo %i\n"RESET, p_inodo_dir);
        return FALLO;
   }

//verificamos que tiene permisos de lectura
   if ((inodo_dir.permisos&2 )!=2){
    mostrar_error_buscar_entrada(ERROR_PERMISO_LECTURA);
    return ERROR_PERMISO_LECTURA;
}

//inicializar el buffer de lectura con 0s
if (memset(buffer,0, BLOCKSIZE/sizeof(struct entrada))==NULL){
    fprintf(stderr, RED "buscar_entrada: Error en el memset()\n"RESET);
    return FALLO;
}

//calculamos la cantidad de entrada que contiene el inodo
cant_entradas_inodo= inodo_dir.tamEnBytesLog/sizeof(struct entrada);
num_entrada_inodo= 0; //nº de entrada inicial

if (cant_entradas_inodo>0){
    if (mi_read_f(*p_inodo_dir, buffer, cant_entradas_inodo+ sizeof(struct entrada), sizeof(entrada))==FALLO){
        fprintf(stderr, RED "buscar_entrada: Error al leer el bloque de entrada\n"RESET);
        return FALLO;
    }
    while (num_entrada_inodo<cant_entradas_inodo && (strcmp(buffer[num_entrada_inodo].nombre, inicial)!=0)){
        //inicializar el buffer de lectura con 0s
    memset(buffer,0, BLOCKSIZE/sizeof(struct entrada));
        num_entrada_inodo++;
       // (*inicial)++;
    }
    memset(buffer,0, BLOCKSIZE/sizeof(struct entrada));


}



if (strcmp(inicial, buffer[num_entrada_inodo].nombre!=0) && num_entrada_inodo== cant_entradas_inodo) {
    switch(reservar){
    case 0: //modo consulta
    mostrar_error_buscar_entrada(ERROR_NO_EXISTE_ENTRADA_CONSULTA);
    return ERROR_NO_EXISTE_ENTRADA_CONSULTA; break;
    case 1://modo escritura

    //si es fichero no permitir escritua
    if (inodo_dir.tipo=='f'){
        mostrar_error_buscar_entrada(ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO);
        return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
    }

    //si es directorio comprobamos que tiene permisos de escritura

    if (inodo_dir.permisos &2 != 2){
        return ERROR_PERMISO_ESCRITURA;

    }else{//tenemos permisos de escritura

        //copiar *inicial en el nombre de la entrada
        if (strcpy(entrada.nombre, *inicial)==NULL){
            fprintf(stderr, RED "buscar_entrada: Error en strcpy()\n"RESET);
            return FALLO;
        }
        if (tipo=='d'){
            if (strcmp(final, "/") == 0) { 
                  //reservamos un inodo directorio y le asiganremos la entrada
                  int ninodo= reservar_inodo('d',permisos);

                  if (ninodo==FALLO){
                    fprintf(stderr, RED"buscar_entrada: Error al reservar el inodo %i para el directorio\n"RESET, ninodo);
                    return FALLO;

                  }
                  buffer[num_entrada_inodo].ninodo=ninodo;
                  

            }else{
                return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
            }
        }else{
            //si es un fichero
            //reservamos un inodo como fichero y le asignamos a la entrada
             int ninodo= reservar_inodo('f',permisos);

                  if (ninodo==FALLO){
                    fprintf(stderr, RED"buscar_entrada: Error al reservar el inodo %i para el fichero \n"RESET, ninodo);
                    return FALLO;

                  }
                  buffer[num_entrada_inodo].ninodo=ninodo;



        }
        //escribir la entrada en el directorio padre
        if (mi_write_f(*p_inodo_dir, &buffer[num_entrada_inodo], BLOCKSIZE/sizeof(struct entrada))==FALLO){
            int ninodo = buffer[num_entrada_inodo].ninodo;
            if (ninodo!=FALLO){// Si se había reservado un inodo para la entrada
            //lo liberamos
                if (liberar_inodo(ninodo)==FALLO){
                    fprintf(stderr, RED "buscar_entrada: Error al liberar el inodo %i reservado previamente\n"RESET, ninodo);
                    return FALLO;
                }

            }
            return FALLO;
        }


    }


    }
    //hemos llegado al final del camin?
    if (strcmp(final, " ")==0 ||strcmp(final, "/")== 0){
        if ((num_entrada_inodo<cant_entradas_inodo) && (reservar==1)){
            //mode escritura y la entrada ya existe
            mostrar_error_buscar_entrada(ERROR_ENTRADA_YA_EXISTENTE);
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
   
        *p_inodo=buffer[num_entrada_inodo].ninodo;
        *p_entrada=num_entrada_inodo;
        return EXITO;
     }else{
    *p_inodo_dir=buffer[num_entrada_inodo].ninodo;
    return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
}
return EXITO;



}

void mostrar_error_buscar_entrada(int error) {
   // fprintf(stderr, "Error: %d\n", error);
   switch (error) {
   case -2: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
   case -3: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
   case -4: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
   case -5: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
   case -6: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
   case -7: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
   case -8: fprintf(stderr, "Error: No es un directorio.\n"); break;
   }
}
