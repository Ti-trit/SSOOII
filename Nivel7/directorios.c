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
