/*
Autores: Khaoula Ikkene, Tomás Gallardo Rago, Francesc Gayá Piña  
Grupo: AntiLinux

*/
#include "ficheros.h"

int main(int argc, char const *argv[]) {

    if (argc!=4){
        fprintf(stderr, RED "Sintaxis: truncar <nombre_dispositivo> <ninodo> <nbytes>\n"RESET);
        return FALLO;
    }

    int ninodo = atoi(argv[2]);
    int nbytes = atoi(argv[3]);
    //montar el dispositivo
    if (bmount(argv[1])==FALLO){
          fprintf(stderr, RED "truncar.c: Error al montar el dispositivo\n"RESET);
            return FALLO;
    }
    if (nbytes==0){
        if (liberar_inodo(ninodo)==FALLO){
            fprintf(stderr, RED "truncar.c: Error al liberar el inodo %i\n" RESET, ninodo);
            return FALLO;
        }
    }else{

        if (mi_truncar_f(ninodo, nbytes)==FALLO){
            fprintf(stderr, RED "truncar_c: Error en mi_truncar_f() del inodo %i\n" RESET, ninodo);
            return FALLO;
        }
    }


    struct STAT p_stat;
    if (mi_stat_f(ninodo, &p_stat)==FALLO){
        fprintf(stderr, RED "truncar.c: Error en mi_stat.f()\n" RESET);
        return FALLO;
    }

    //imprimos la información de p_stat
    
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];


    ts = localtime(&p_stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);

    //  tiempo de última modificación 
    ts = localtime(&p_stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);

    // tiempo de cambio del estado del pstat
    ts = localtime(&p_stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    fprintf(stdout, "\nDATOS INODO %d:\n", ninodo);
    fprintf(stdout, "tipo=%c\n", p_stat.tipo);
    fprintf(stdout, "permisos=%d\n", p_stat.permisos);
    fprintf(stdout, "atime: %s\n", atime);
    fprintf(stdout, "ctime: %s\n", ctime);
    fprintf(stdout, "mtime: %s\n", mtime);
    fprintf(stdout, "nLinks=%d\n", p_stat.nlinks);
    fprintf(stdout, "tamEnBytesLog=%d\n", p_stat.tamEnBytesLog);
    fprintf(stdout, "numBloquesOcupados=%d\n", p_stat.numBloquesOcupados);
    
    

  
    if (bumount() == FALLO) {
        fprintf(stderr, RED"truncar.c: Error al desmontar el dispositivo.\n"RESET);
        return FALLO;
    }
return EXITO;
}