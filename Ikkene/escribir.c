/**
Autores: Khaoula Ikkene 
**/
#include <stdio.h>
#include "ficheros.h"

int main(int argc, char const *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, RED "Sintaxis: escribir <nombre_dispositivo> <'$(cat fichero)'> <diferentes_inodos>\n");
        fprintf(stderr, RED "Offsets: 9000, 209000, 30725000, 409605000, 480000000 \n");
        fprintf(stderr, RED "Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets\n" RESET);
        return FALLO;
    }

    int offset[5] = {9000, 209000, 30725000, 409605000, 480000000}; // offsets

    //montar el dispositivo virtual
    if (bmount(argv[1]) == FALLO)
    {
        fprintf(stderr, RED "Error al montar el dispositivo\n" RESET);
        return FALLO;
    }
    //reservar el primer inodo
    int ninodo = reservar_inodo('f', 6);
    if (ninodo == FALLO)
    {
        fprintf(stderr, RED "Error al reservar el inodo\n" RESET);
        return FALLO;
    }
       printf("Longitud texto: %ld \n\n", strlen(argv[2]));


    for (int i = 0; i < 5; i++)
    {
        printf("Numero inodo reservado: %d\n", ninodo);
        printf("Offset: %d\n", offset[i]);

        int bytesEscritos = mi_write_f(ninodo, argv[2], offset[i], strlen(argv[2]));
        if (bytesEscritos == FALLO)
        {
            fprintf(stderr, RED "Error en mi_write_f() (escribir.c\n" RESET);
            return FALLO;
        }
        /*int tamaño =strlen(argv[2]);
        char buf[tamaño];
       
        if(memset(buf, 0, strlen(argv[2])) == NULL){
            fprintf(stderr, RED"Error en escribir.c\n"RESET);
            return FALLO;
        }
        unsigned int bytesLeidos = 0;
        bytesLeidos = mi_read_f(ninodo, buf, offset[i], tamaño);
        if(bytesLeidos == FALLO){
            fprintf(stderr, RED"Error en mi_read_f() (escribir.c)\n"RESET);
            return FALLO;
        }
        fprintf(stdout, "Bytes leídos: %u\n", bytesLeidos);
        */
        struct STAT stat;
        if (mi_stat_f(ninodo, &stat) == FALLO)
        {
            fprintf(stderr, RED "Error main mi_stat_f\n" RESET);
            return FALLO;
        }
       
        fprintf(stdout,"Bytes escritos: %d \n", bytesEscritos);
        fprintf(stdout,"stat.tamEnBytesLog = %d\n", stat.tamEnBytesLog);
        fprintf(stdout,"stat.numBloquesOcupados = %d\n\n", stat.numBloquesOcupados);

        if (strcmp(argv[3], "0"))
        {//si reserva un solo inodo para todos los offsets
            ninodo = reservar_inodo('f', 6);
            if (ninodo == FALLO)
            {
                fprintf(stderr, RED "Error al reservar el inodo\n" RESET);
                return FALLO;
            }
            
        }
    }

    if (bumount() == FALLO)
    {
        fprintf(stderr, RED "Error main bumount\n" RESET);
    }
}