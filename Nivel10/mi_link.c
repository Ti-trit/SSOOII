#include "directorios.h"
/**
 * Programa que crea un enlace a un fichero , llamando a mi_link()
*/

int main(int argc, char *argv[]) {
    // Comprobar el número correcto de argumentos
        if (argc != 4) {
            printf("Sintaxis: ./mi_link disco /ruta_fichero_original /ruta_enlace\n");
            return FALLO;
        }
        // Obtenemos los argumentos
    const char *disco = argv[1];
    const char *ruta_fichero_original = argv[2];
    const char *ruta_enlace = argv[3];

    // Comprueba que las rutas sean válidas
    struct stat file_stat;
    if (stat(ruta_fichero_original, &file_stat) != 0 || !S_ISREG(file_stat.st_mode)) {
        printf("Error: %s no es un archivo válido.\n", ruta_fichero_original);
        return FALLO;
    }

    if (stat(ruta_enlace, &file_stat) == 0) {
        printf("Error: El archivo %s ya existe.\n", ruta_enlace);
        return FALLO;
    }

    // Crear el enlace
    if (mi_link(ruta_fichero_original, ruta_enlace) != 0) {
        perror("Error al crear el enlace");
        return FALLO;
    }
    return EXITO; 
}
