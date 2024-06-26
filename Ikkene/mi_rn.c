/**
 * @author Khaoula Ikkene
 **/
#include "directorios.h"
/**
 * Programa que renombra un fichero/directorio llamando a la función mi_rn de la capa
 * de directorios.
 *
 */

int main(int argc, char **argv)
{

    if (argc != 4)
    {
        fprintf(stderr, RED "Sintaxis: $ ./mi_rn <disco> </ruta/antiguo> <nuevo> \n " RESET);
        return FALLO;
    }

    // montar el disco

    if (bmount(argv[1]) == FALLO)
    {
        fprintf(stderr, RED "mi_rn.c: Error al montar el disco\n" RESET);
        return FALLO;
    }

    char *rutaAntigua = argv[2];
    char *nuevo = argv[3];
    int error = mi_rn(rutaAntigua, nuevo);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return error;
    }

    // desmontar el disco
    if (bumount() == FALLO)
    {
        fprintf(stderr, RED "mi_rn: Error al desmontar el" RESET);
        return FALLO;
    }
    return EXITO;
}