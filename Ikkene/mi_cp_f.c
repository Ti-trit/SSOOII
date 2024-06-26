/**
 * @author Khaoula Ikkene
 **/
#include "directorios.h"
/**
 * Programa que copia un fichero/directorio en otro directorio
 *
 *
 */

int main(int argc, char **argv)
{

    if (argc != 4)
    {
        fprintf(stderr, RED "Sintaxis: $ ./mi_cp_f <disco> </origen/nombre> </destino/>\n" RESET);
        return FALLO;
    }

    // montar el disco

    if (bmount(argv[1]) == FALLO)
    {
        fprintf(stderr, RED "mi_cp_f.c: Error al montar el disco\n" RESET);
        return FALLO;
    }

    char *rutaAntigua = argv[2];
    char *destino = argv[3];
    int error = mi_cp_f(rutaAntigua, destino);
    if (error < 0)
    {
        mostrar_error_buscar_entrada(error);
        return error;
    }

    // desmontar el disco
    if (bumount() == FALLO)
    {
        fprintf(stderr, RED "mi_cp_f.c: Error al desmontar el" RESET);
        return FALLO;
    }
    return EXITO;
}