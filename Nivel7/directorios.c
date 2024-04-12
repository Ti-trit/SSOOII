#include "directorios.h"

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {
    if (camino[0] != '/') {
        fprintf(stderr, RED"Error, el camino no contiene / al inicio\n" RESET);
        return FALLO;
    }

    const char *segundo_slash = strchr(camino + 1, '/');
    if (segundo_slash != NULL) {
        // Extraer la posicion hasta el segundo '/'
        int len = segundo_slash - (camino + 1);
        strncpy(inicial, camino + 1, len);
        inicial[len] = '\0';

        //Resto del camino
        strcpy(final, segundo_slash);
        *tipo = 'd'; //Es un directorio porque hay un segundo '/'
    } else {
        //No hay segunco '/', por lo tanto es un fichero
        strcpy(inicial, camino + 1);
        final[0] = '\0';
        *tipo = 'f';
    }

    return EXITO;
}