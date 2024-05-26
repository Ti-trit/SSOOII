#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <verificacion.h>

#define NUMPROCESOS 100
#define N 100

// Definimos struct REGISTRO 
struct REGISTRO {
    int pid;
    unsigned int nEscritura;
    unsigned int nRegistro;
    time_t tiempo; fecha
};

// Función para convertir el tiempo en una cadena legible
char *formatearTiempo(time_t tiempo) {
    static char buffer[30];
    strftime(buffer, sizeof(buffer), "%a %d-%m-%Y %H:%M:%S", localtime(&tiempo));
    return buffer;
}

// Función principal
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <nombre_dispositivo> <directorio_simulación>\n", argv[0]);
        return 1;
    }

    char *nombreDispositivo = argv[1];
    char *directorioSimulacion = argv[2];

    //montar el dispositivo virtual
    if (bmount(argv[1]) == FALLO)
    {
        fprintf(stderr, RED "Error al montar el dispositivo\n" RESET);
        return FALLO;
    }

    struct stat st;
    if (stat(directorioSimulacion, &st) == -1) {
        perror("Error al obtener información del directorio de simulación");
        return FALLO;
    }

    int numentradas = st.st_nlink - 2; // Restar 2 para . y ..
    if (numentradas != NUMPROCESOS) {
        fprintf(stderr, "ERROR: El número de entradas del directorio (%d) no es igual a NUMPROCESOS (%d)\n", numentradas, NUMPROCESOS);
        return FALLO;
    }

    char informePath[256];
    snprintf(informePath, sizeof(informePath), "%s/informe.txt", directorioSimulacion);
    FILE *informe = fopen(informePath, "w");
    if (!informe) {
        perror("Error al crear el archivo informe.txt");
        return FALLO;
    }

    struct dirent *entry;
    DIR *dp = opendir(directorioSimulacion);
    if (!dp) {
        perror("Error al abrir el directorio de simulación");
        return FALLO;
    }

    while ((entry = readdir(dp))) {
        if (entry->d_type == DT_DIR && strncmp(entry->d_name, "proceso_", 8) == 0) {
            int pid = atoi(entry->d_name + 8);
            char filePath[256];
            snprintf(filePath, sizeof(filePath), "%s/%s/prueba.dat", directorioSimulacion, entry->d_name);
            
            FILE *file = fopen(filePath, "rb");
            if (!file) {
                perror("Error al abrir prueba.dat");
                continue;
            }

            struct INFORMACION info;
            info.pid = pid;
            info.nEscrituras = 0;
            struct REGISTRO buffer[N];
            int isFirst = 1;

            while (fread(buffer, sizeof(struct REGISTRO), N, file) > 0) {
                for (int i = 0; i < N; i++) {
                    if (buffer[i].pid == pid) {
                        if (isFirst) {
                            info.PrimeraEscritura = buffer[i];
                            info.UltimaEscritura = buffer[i];
                            info.MenorPosicion = buffer[i];
                            info.MayorPosicion = buffer[i];
                            isFirst = 0;
                        } else {
                            if (buffer[i].nEscritura < info.PrimeraEscritura.nEscritura) {
                                info.PrimeraEscritura = buffer[i];
                            }
                            if (buffer[i].nEscritura > info.UltimaEscritura.nEscritura) {
                                info.UltimaEscritura = buffer[i];
                            }
                            if (buffer[i].nRegistro < info.MenorPosicion.nRegistro) {
                                info.MenorPosicion = buffer[i];
                            }
                            if (buffer[i].nRegistro > info.MayorPosicion.nRegistro) {
                                info.MayorPosicion = buffer[i];
                            }
                        }
                        info.nEscrituras++;
                    }
                }
            }

            fclose(file);

            fprintf(informe, "PID: %d\n", info.pid);
            fprintf(informe, "Numero de escrituras: %d\n", info.nEscrituras);
            fprintf(informe, "Primera Escritura\t%d\t%d\t%s\n", info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, formatearTiempo(info.PrimeraEscritura.tiempo));
            fprintf(informe, "Ultima Escritura\t%d\t%d\t%s\n", info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, formatearTiempo(info.UltimaEscritura.tiempo));
            fprintf(informe, "Menor Posición\t%d\t%d\t%s\n", info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, formatearTiempo(info.MenorPosicion.tiempo));
            fprintf(informe, "Mayor Posición\t%d\t%d\t%s\n\n", info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, formatearTiempo(info.MayorPosicion.tiempo));

            printf("[%d) %d escrituras validadas en %s]\n", pid, info.nEscrituras, filePath);
        }
    }

    closedir(dp);
    fclose(informe);

    // Desmontar el dispositivo virtual (esto también dependerá de su implementación)
    if (umount(directorioSimulacion) == -1) {
        perror("Error al desmontar el dispositivo");
        return FALLO;
    }

    return 0;
}
