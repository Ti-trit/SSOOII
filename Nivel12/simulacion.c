#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "simulacion.h"

static int acabados = 0;


void reaper() {
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0) {
        acabados++;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <disco>\n", RESET);
        return FALLO;
    }

    //Crear el directorio simulación

    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, RED "mi_touch.c: Error al montar el dispositivo virtual\n" RESET);
        return FALLO;
    }

    for (int i = 0; i < NUMPROCESOS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            if (bmount(argv[1]) == FALLO) {
                fprintf(stderr, RED "mi_touch.c: Error al montar el dispositivo virtual\n" RESET);
                return FALLO;
            }

            // Crear el directorio del proceso hijo 


            // Crear el fichero prueba.dat
            char ficheroPrueba[256];
            sprintf(ficheroPrueba, "%s/prueba.dat", dirProceso);
            int fd = open(ficheroPrueba, O_CREAT | O_WRONLY, 0644);
            if (fd == -1) {
                fprintf(stderr, RED "ERror al crear el fichero prueba.dat\n" RESET);
                return FALLO;
            }

            srand(time(NULL) + getpid());

            for (int j = 0; j < NUMESCRITURAS; j++) {
                struct REGISTRO registro;
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = j + 1;
                registro.nRegistro = rand() % REGMAX;

                if (lseek(fd, registro.nRegistro * sizeof(struct REGISTRO), SEEK_SET) == -1) {
                    fprintf(stderr, RED "Error en lseek\n" RESET);
                    return FALLO;
                }

                if (mi_write(fd, &registro, sizeof(struct REGISTRO)) == -1) {
                    fprintf(stderr, RED "Error al escribir en el fichero\n" RESET);
                    return FALLO
                }

                usleep(50000); // 0.05 segundos
            }

            if (bumount() == FALLO){
                fprintf(stderr, RED "Error al desmontar el dispositivo virtual.\n" RESET);
                return FALLO;
            }
            exit(0);
        }

        usleep(150000); // 0.15 segundos
    }

    while (acabados < NUMPROCESOS) {
        pause();
    }

    if (bumount() == FALLO){
        fprintf(stderr, RED "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    exit(0);
}
