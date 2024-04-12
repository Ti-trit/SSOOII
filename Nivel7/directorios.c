#include "directorios.h"
/**
 * @brief Función que separa una cadena en dos partes, donde la parte inicial representa
 * un directorio.
 * @param   camino    cadena a separar
 * @param   inicial   parte inicial obtenido de camino
 * @param   final     resta de la cadena camino
 * @param   tipo      tipo del camino
 * @return            EXITO o FALLO  
*/
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {
    if (camino[0] != '/') {
        fprintf(stderr, RED"Error, el camino no contiene / al inicio\n" RESET);
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
