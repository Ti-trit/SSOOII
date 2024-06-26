/**
Autores: Khaoula Ikkene, Tomás Gallardo Rago, Francesc Gayá Piña  
**/
#include "directorios.h"

int main(int argc, char const *argv[]) {
    // Comprobar sintaxis
    if (argc != 3) {
        fprintf(stderr, "Sintaxis: %s <disco> </ruta>\n", argv[0]);
        return FALLO;
    }

    struct STAT p_stat;
    
    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, RED "mi_stat.c: Error al montar el dispositivo\n"RESET);
        return FALLO;
    }
    int p_inodo = mi_stat(argv[2], &p_stat);
    if(p_inodo < 0) {
        mostrar_error_buscar_entrada(p_inodo);
        return FALLO;
    }
    
    struct tm *ts;
    char atime[80], mtime[80], ctime[80];

    ts = localtime(&p_stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&p_stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&p_stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    fprintf(stdout, "Nº de inodo: %d\n", p_inodo);
    fprintf(stdout, "tipo: %c\n", p_stat.tipo);
    fprintf(stdout, "permisos: %d\n", p_stat.permisos);
    fprintf(stdout, "atime: %s\n", atime);
    fprintf(stdout, "ctime: %s\n", ctime);
    fprintf(stdout, "mtime: %s\n", mtime);
    fprintf(stdout, "nlinks: %d\n", p_stat.nlinks);
    fprintf(stdout, "tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
    fprintf(stdout, "numBloquesOcupados: %d\n", p_stat.numBloquesOcupados);

    if (bumount() ==  FALLO) {
        mostrar_error_buscar_entrada(FALLO);
        return FALLO;
    }
    
    return EXITO; 
}