

#include "simulacion.h"

unsigned char acabados = 0;

//enterrador de procesos hijos
void reaper() {
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0) {
        acabados++;
    }
}

int main(int argc, char const *argv[]) {
    //Asociar sigchld al reaper()
    signal(SIGCHLD, reaper);

    // Comprobamos la sintaxis
    if (argc != 2) {
        fprintf(stderr, RED "Sintaxis: ./simulacion <disco>\n" RESET);
        return FALLO;
    }
        if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, RED "Error al montar el dispositivo virtual\n" RESET);
        return FALLO;
    }

    int error;
   
    time_t aux_time = time(NULL);
    struct tm tiempo = *localtime(&aux_time);
    char * tiempo_buf = malloc(14);//4(year)+ 2(month)+2(day)+2(hour)+2(minut)+2(second)
    sprintf(tiempo_buf, "%d%02d%02d%02d%02d%02d", tiempo.tm_year + 1900, 1 + tiempo.tm_mon, tiempo.tm_mday, tiempo.tm_hour, tiempo.tm_min, tiempo.tm_sec);
   
    char *ruta = malloc(22); //14 + 6 (/simul_)+ 1(/) --> Longitud fija para la ruta con el formato "/simul_aaaammddhhmmss/"

    memset(ruta, 0, strlen(ruta));
    strcpy(ruta, "/simul_");
    strcat(ruta, tiempo_buf); //concatenar el tiempo
    strcat(ruta, "/"); //simul_aaaammddhhmmss/

    char buffer[80];
    strcpy(buffer,ruta);   
    error = mi_creat(ruta, 6);
    if (error < 0) {
        if (error == FALLO) {
            fprintf(stderr, RED "simulacion.c: Error al crear el directorio de simulación\n" RESET);
        } else {
            mostrar_error_buscar_entrada(error);
           // fprintf(stderr, "ERROR EN EL PRIMER CREAT\n");
        }
        return FALLO;
    }
    fprintf(stdout, "*** SIMULACIÓN DE %i PROCESOS REALIZANDO CADA UNO %i ESCRITURAS ***:\n", NUMPROCESOS, NUMESCRITURAS);
    for (unsigned char i = 1; i <= NUMPROCESOS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            if (bmount(argv[1]) == FALLO) {
                fprintf(stderr, RED "Error al montar el dispositivo virtual\n" RESET);
                exit(-1);
                //return FALLO;
            }
            char ruta_hijo[100]; 
            sprintf(ruta_hijo, "%sproceso_%d/", buffer, getpid());
            error = mi_creat(ruta_hijo, 6);
            if (error < 0) {
                if (error == FALLO) {
                    fprintf(stderr, RED "Error al crear un directorio de un proceso hijo\n" RESET);
                  //  fprintf(stderr, YELLOW"Error en el creat del hijo %s"RESET, ruta_hijo);
                } else {
                    mostrar_error_buscar_entrada(error);
                }
                exit(-1);
                //return FALLO;
            }
            strcat(ruta_hijo, "prueba.dat");
            error = mi_creat(ruta_hijo, 6);//crear el fichero con permisos de lectura y escritura
            if (error < 0) {
                if (error == FALLO) {
                    fprintf(stderr, RED "Error al crear fichero de un proceso hijo\n" RESET);
                } else {
                    mostrar_error_buscar_entrada(error);
                }
                exit(-1);
                //return FALLO;
            }
            // se consigue semilla a partir de valor aleatorio
            srand(time(NULL) + getpid());
            for (unsigned char escritura = 1; escritura <= NUMESCRITURAS; escritura++) {
                struct REGISTRO registro;
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = escritura;
                registro.nRegistro = rand()%REGMAX;
                error = mi_write(ruta_hijo, &registro, registro.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO));
                if (error < 0) {
                    if (error == FALLO) {
                        fprintf(stderr, RED "Error al escribir el registro %i\n" RESET, registro.nRegistro);
                    } else {
                        mostrar_error_buscar_entrada(error);
                    }
                    exit(-1);
                }
#if DEBUGSIMUL
                fprintf(stderr, GRAY "[simulación.c → Escritura %i en %s]\n" RESET, nescritura, ruta_hijo);
#endif
                usleep(50000);
            }
#if DEBUGN12
            fprintf(stderr, BLUE "[Proceso %i: Completadas %i escrituras en %s]\n" RESET, i, NUMESCRITURAS, ruta_hijo);
#endif
            if (bumount() == FALLO) {
                fprintf(stderr, RED "Error al desmontar el dispositivo virtual.\n" RESET);
                exit(-1);
            }
            exit(0);
        }
        usleep(200000);
    } 

    while (acabados < NUMPROCESOS) {
        pause();
    }

    if (bumount() == FALLO) {
        fprintf(stderr, RED "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    return EXITO;
}


