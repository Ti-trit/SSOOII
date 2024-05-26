// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <dirent.h>
// #include <sys/stat.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <time.h>
#include "verificacion.h"

// #define NUMPROCESOS 100
// #define N 100

// // Definimos struct REGISTRO 
// struct REGISTRO {
//     int pid;
//     unsigned int nEscritura;
//     unsigned int nRegistro;
//     time_t tiempo; fecha
// };

/**
 * 
 * Primera escritura: registro con el nº de escritura menor.
 *  Última escritura: registro con el nº de escritura mayor.
 *  Menor posición: registro con la posición (nº registro) más baja.
 *  Mayor posición: registro con la posición (nº registro) más alta.
*/

// Función para convertir el tiempo en una cadena legible
char *formatearTiempo(time_t tiempo) {
    static char buffer[80];
    strftime(buffer, sizeof(buffer), "%a %d-%m-%Y %H:%M:%S", localtime(&tiempo));
    return buffer;
}

// Función principal
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, RED"Uso: %s <nombre_dispositivo> <directorio_simulación>\n"RESET, argv[0]);
        return FALLO;
    }

  //  char *nombreDispositivo = argv[1];
    char *directorioSimulacion = argv[2];

    //montar el dispositivo virtual
    if (bmount(argv[1]) == FALLO)
    {
        fprintf(stderr, RED "Error al montar el dispositivo\n" RESET);
        return FALLO;
    }

    struct STAT st;
    int error = mi_stat(directorioSimulacion, &st);
    if (error < 0) {
        // perror("Error al obtener información del directorio de simulación");
        // return FALLO;
        if (error == FALLO) {
            fprintf(stderr, RED "verificacion.c:Error al obtener información del directorio de simulació\n" RESET);
        } else {
            mostrar_error_buscar_entrada(error);
        }
        return FALLO;
    }

  //  int numentradas = st.st_nlink - 2; // Restar 2 para . y ..
  int numentradas = st.tamEnBytesLog/sizeof(struct entrada);
    if (numentradas != NUMPROCESOS) {
        fprintf(stderr, "ERROR: El número de entradas del directorio (%d) no es igual a NUMPROCESOS (%d)\n", numentradas, NUMPROCESOS);
        return FALLO;
    }

    fprintf(stdout, "dir_sim: %s\n", argv[1]);
    char informePath[256];

    // snprintf(informePath, sizeof(informePath), "%s/informe.txt", directorioSimulacion);
    // FILE *informe = fopen(informePath, "w");
    // if (!informe) {
    //     perror("Error al crear el archivo informe.txt");
    //     return FALLO;
    // }

    sprintf(informePath, "%sinforma.txt", directorioSimulacion);
    // Crear el fichero "informe.txt" dentro del directorio de simulación.
    if (mi_creat(informePath, 6)<0){
        return FALLO;
    }

    // struct dirent *entry;
    // DIR *dp = opendir(directorioSimulacion);
    // if (!dp) {
    //     perror("Error al abrir el directorio de simulación");
    //     return FALLO;
    // }

        //leemos las entradas de los directorios
        struct entrada directorios[NUMPROCESOS*sizeof(struct entrada)];
        if (mi_read(directorioSimulacion, directorios, 0, numentradas*sizeof(struct entrada))<0){
            fprintf(stderr, RED "verificacion.c: Error al leer las entradas de los directorios\n"RESET);
            return FALLO;
        }

    fprintf(stdout, "numentradas: %i NUMPROCESOS: %i\n", numentradas, NUMPROCESOS);

    //Para cada entrada extraemos el PID y lo guardamos en el registro info
    for (int i = 0; i<numentradas; i++){

            //obtenemos la entrada del directorio i
            char * Nombreentrada = directorios[i].nombre;
            char *pidchar = strchr(Nombreentrada,'_');
            pid_t pid = atoi(pidchar);



  //  }
    // while ((entry = readdir(dp))) {
    //     if (entry->d_type == DT_DIR && strncmp(entry->d_name, "proceso_", 8) == 0) {
    //         int pid = atoi(entry->d_name + 8);
    //         char filePath[256];
    //         snprintf(filePath, sizeof(filePath), "%s/%s/prueba.dat", directorioSimulacion, entry->d_name);
            
    //         FILE *file = fopen(filePath, "rb");
    //         if (!file) {
    //             perror("Error al abrir prueba.dat");
    //             continue;
            // }

            struct INFORMACION info;
            info.pid = pid;
            info.nEscrituras = 0;
           // struct REGISTRO buffer[N];
          //  int isFirst = 1;

            //camino de nuestro fichero
            char fichero_proceso[128];
            //nombre del fichero es PID+preuba.dat
            sprintf(fichero_proceso,"%s%sprueba.dat", directorioSimulacion, Nombreentrada);

            int cant_registros_buffer_escrituras = 256, offset= 0;
            struct REGISTRO buffer_escrituras [cant_registros_buffer_escrituras];
            memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
            while (mi_read(fichero_proceso, buffer_escrituras, offset, sizeof(buffer_escrituras)) > 0){
                //leer una escritura

                for (int i = 0; i<cant_registros_buffer_escrituras; i++){
                         //la escritura e valida?
                         if (buffer_escrituras[i].pid==info.pid) {

                        //es la primera escritura valida?
                            if (info.nEscrituras==0){
                        //Inicializar los registros significativos con los datos de esa escritura. 
                            info.MayorPosicion=buffer_escrituras[i];
                            info.UltimaEscritura = buffer_escrituras[i];
                            info.MenorPosicion = buffer_escrituras[i];
                            info.PrimeraEscritura = buffer_escrituras[i];
                        //ya será la de menor posición puesto que hacemos un barrido secuencial
                            }else{
                // Comparar nº de escritura (para obtener primera y última) y actualizarlas si es preciso
                            if(difftime(buffer_escrituras[i].fecha, info.PrimeraEscritura.fecha)<=0 && buffer_escrituras->nEscritura<info.PrimeraEscritura.nEscritura){
                                info.PrimeraEscritura= buffer_escrituras[i];
                            }
                            if (difftime(buffer_escrituras[i].fecha, info.UltimaEscritura.fecha)>0 && buffer_escrituras->nEscritura>info.PrimeraEscritura.nEscritura){
                                 info.UltimaEscritura= buffer_escrituras[i];    

                            }

                            if (buffer_escrituras[i].nRegistro<info.MenorPosicion.nRegistro){
                                info.MenorPosicion=buffer_escrituras[i];

                            }
                            if (buffer_escrituras[i].nRegistro>info.MayorPosicion.nRegistro){
                                 info.MayorPosicion=buffer_escrituras[i];
                            }
                         }
                         //incrementar nº escrituras validas
                         info.nEscrituras++;

                } 
            }
            //Hay que limpiar el buffer de lectura antes de cada nuevo uso!!!
            memset(&buffer_escrituras, 0, sizeof(buffer_escrituras));
            offset += sizeof(buffer_escrituras);//actualizar el offset en cada iteración
            //obtener la escritura de la úlitma posición
           

        // char tiempo_PrimeraEscritura[80];
        // char tiempo_UltimaEscritura[80];
        // char tiempo_Menor[80];
        // char tiempo_Mayor[80];
        // struct tm *tm;

        // tm = localtime(&info.PrimeraEscritura.fecha);
        // strftime(tiempo_PrimeraEscritura, sizeof(tiempo_PrimeraEscritura), "%a %Y-%m-%d %H:%M:%S", tm);
        // tm = localtime(&info.UltimaEscritura.fecha);
        // strftime(tiempo_UltimaEscritura, sizeof(tiempo_UltimaEscritura), "%a %Y-%m-%d %H:%M:%S", tm);
        // tm = localtime(&info.MenorPosicion.fecha);
        // strftime(tiempo_Menor, sizeof(tiempo_Menor), "%a %Y-%m-%d %H:%M:%S", tm);
        // tm = localtime(&info.MayorPosicion.fecha);
        // strftime(tiempo_Mayor, sizeof(tiempo_Mayor), "%a %Y-%m-%d %H:%M:%S", tm);

            char informe[BLOCKSIZE];
            memset(informe, 0, BLOCKSIZE);
            sprintf(informe, "PID: %i\nNumero de escrituras: %i\n", pid, info.nEscrituras);

            sprintf(informe, "PID: %d\n", info.pid);
            sprintf(informe, "Numero de escrituras: %d\n", info.nEscrituras);
            sprintf(informe, "Primera Escritura\t%d\t%d\t%s\n", info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, formatearTiempo(info.PrimeraEscritura.fecha));
            sprintf(informe, "Ultima Escritura\t%d\t%d\t%s\n", info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, formatearTiempo(info.UltimaEscritura.fecha));
            sprintf(informe, "Menor Posición\t%d\t%d\t%s\n", info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, formatearTiempo(info.MenorPosicion.fecha));
            sprintf(informe, "Mayor Posición\t%d\t%d\t%s\n\n", info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, formatearTiempo(info.MayorPosicion.fecha));
            strcat(informe, "\n");
 
        fprintf(stderr, GRAY "[%i) %i escrituras validadas en %s]\n" RESET, i+1, info.nEscrituras, informePath);
        //  Añadir la información del struct info al fichero informe.txt por el final.
        int error = mi_write(informePath, &informe, 0, strlen(informe));
        if (error<0){
            mostrar_error_buscar_entrada(error);
            return FALLO;
        }
        }
    }


    // Desmontar el dispositivo virtual
    if (bumount() == FALLO) {
        fprintf(stderr, RED"verificacion.c: Error al desmontar el dispositivo.\n"RESET);
        return FALLO;
    }

    return 0;
}
