#include "simulacion.h"

static int acabados = 0;

void reaper()
{
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        acabados++;
    }
}

int main(int argc, char *argv[])
{
    // asigamons la señal SIGCHLD sl reaper
    signal(SIGCHLD, reaper);

    if (argc != 2)
    {
        fprintf(stderr, RED"Uso:./simulacion <disco>\n" RESET);
        return FALLO;
    }

    // montar el disco virtual
    if (bmount(argv[1]) == FALLO)
    {
        fprintf(stderr, RED "Error al montar el dispositivo virtual\n" RESET);
        return FALLO;
    }

    // creamos el directorio de somulación /simul_aaaammddhhmmss/

time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char *tiempo = malloc(14);//14 = 4(año)+2(mes)+2(dia)+2(hora)+2(segundos)
    sprintf(tiempo, "%d%02d%02d%02d%02d%02d", tm.tm_year + 1900,
            tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    char *camino = malloc(14 + 8);//6 (simul_)+ 2(/.../)
    strcat(strcpy(camino, "/simul_"), tiempo);
    strcat(camino, "/");
    fprintf(stderr, "Directorio de simulación:%s \n", camino);

    char buf[14+8];
    strcpy(buf, camino);

    fprintf(stderr, "Buffer de simulación:%s \n", buf);
        int error = (mi_creat(camino, 6));
    if (error < 0)
    {
        if (error == FALLO){
            fprintf(stderr, RED "Error al crear el directorio de simulación\n"RESET);
        }else{
        mostrar_error_buscar_entrada(error);
        }
        return FALLO;
    }
   // fprintf(stderr, "Directorio creado\n");
    fprintf(stdout, "*** SIMULACIÓN DE %i PROCESOS REALIZANDO CADA UNO %i ESCRITURAS ***:\n", NUMPROCESOS, NUMESCRITURAS);

    for (int i = 1; i <= NUMPROCESOS; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        { // si es el hijo
            if (bmount(argv[1]) == FALLO)
            {
                fprintf(stderr, RED "Error al montar el dispositivo virtual\n" RESET);
                return FALLO;
            }

            // Crear el directorio del proceso hijo

            // Crear el fichero prueba.dat
            char ficheroPrueba[80];
            sprintf(ficheroPrueba, "%d/prueba.dat", getpid());
            int fd = mi_creat(ficheroPrueba, 4);
            if (fd <0)
            {
               // fprintf(stderr, RED "ERror al crear el fichero prueba.dat\n" RESET);
               mostrar_error_buscar_entrada(fd);
               fprintf(stderr, "Error en el primer creat");
              // bumount();
                return FALLO;
            }
            //  Inicializar la semilla de números aleatorios
            srand(time(NULL) + getpid());

            for (int j = 1; j <= NUMESCRITURAS; j++)
            { // 50
                // inicializar el registro
                struct REGISTRO registro;
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = j;
                registro.nRegistro = rand() % REGMAX; //[0, 499.999]

                // escribimos el registro
                //  if (lseek(fd, registro.nRegistro * sizeof(struct REGISTRO), SEEK_SET) == -1) {
                //      fprintf(stderr, RED "Error en lseek\n" RESET);
                //      return FALLO;
                //  }

                int error = mi_write(ficheroPrueba, &registro, registro.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO));
                if (error < 0)
                {
                    // fprintf(stderr, RED "Error al escribir en el fichero\n" RESET);
                    mostrar_error_buscar_entrada(error);
                    return FALLO;
                }
#if DEBUGN12
                fprintf(stderr, "[simulación.c → Escritura %i en %s]\n", j, buf);
#endif

                usleep(50000); // 0.05 segundos
            } // fpara

            // desmontamos el disco virtual del hijo
            if (bumount() == FALLO)
            {
                fprintf(stderr, RED "Error al desmontar el dispositivo virtual.\n" RESET);
                return FALLO;
            }
            exit(0); // Necesario para que se emita la señal SIGCHLD
        }

        usleep(150000); // 0.15 segundos
    } // fpara
    // Permitir que el padre espere por todos los hijos
    while (acabados < NUMPROCESOS)
    {
        pause();
    }

    if (bumount() == FALLO)
    {
        fprintf(stderr, RED "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }
        fprintf(stderr, "Total de procesos terminados: %d\n", acabados);

    exit(0); // o return EXIT
}
