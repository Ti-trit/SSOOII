
#include "verificacion.h"

// Función auxiliar para convertir el tiempo en una cadena legible
char *formatearTiempo(time_t tiempo)
{
    static char buffer[80];
    
    strftime(buffer, sizeof(buffer), "%a %d-%m-%Y %H:%M:%S", localtime(&tiempo));
    return buffer;
}

int main(int argc, char *argv[])
{
    //comprobamos la sintaxis
    if (argc != 3)
    {
        fprintf(stderr, RED "Uso: %s <nombre_dispositivo> <directorio_simulación>\n" RESET, argv[0]);
        return FALLO;
    }

    char *directorioSimulacion = argv[2];

    // montar el dispositivo virtual
    if (bmount(argv[1]) == FALLO)
    {
        fprintf(stderr, RED "Error al montar el dispositivo\n" RESET);
        return FALLO;
    }

    struct STAT st;
    int error = mi_stat(directorioSimulacion, &st);
    if (error < 0)
    {
        fprintf(stderr, RED "verificacion.c: Error al obtener información del directorio de simulación:  %s\n" RESET, directorioSimulacion);
        return FALLO;
    }

    int numentradas = st.tamEnBytesLog / sizeof(struct entrada);
    if (numentradas != NUMPROCESOS)
    {
        fprintf(stderr, RED "mi_verificacion.c: El número de entradas es diferente a NUMPROCESOS\n" RESET);
        return FALLO;
    }

    fprintf(stdout, "dir_sim: %s\n", directorioSimulacion);
    char informePath[256];

    sprintf(informePath, "%sinforme.txt", directorioSimulacion);
    // Crear el fichero "buffer_informe.txt" dentro del directorio de simulación.
    if (mi_creat(informePath, 6) < 0)
    {
        return FALLO;
    }

    // leemos las entradas de los directorios
    struct entrada directorios[numentradas];
    if (mi_read(directorioSimulacion, directorios, 0, numentradas * sizeof(struct entrada)) < 0)
    {
        fprintf(stderr, RED "verificacion.c: Error al leer las entradas de los directorios\n" RESET);
        return FALLO;
    }

    fprintf(stdout, "numentradas: %i NUMPROCESOS: %i\n", numentradas, NUMPROCESOS);

    int bytes_escritos = 0;
    // Para cada entrada extraemos el PID y lo guardamos en el registro info
    for (int i = 0; i < numentradas; i++){
        struct INFORMACION info;
        // obtenemos la entrada del directorio i
        char *Nombreentrada = directorios[i].nombre;
        char *pidchar = strchr(Nombreentrada, '_'); 
        pid_t pid = atoi(pidchar+1);

        info.pid = pid;
        info.nEscrituras = 0;
        // camino de nuestro fichero
        char rutaPrueba[128];
        // nombre del fichero es PID+preuba.dat
        sprintf(rutaPrueba, "%s%s/prueba.dat", directorioSimulacion, Nombreentrada);
        // fprintf(stdout, "fichero hijo %s\n", rutaPrueba);
        //la cant_registros_buffer_escrituras debe ser multiplo de BLOCKSIZE
        int cant_registros_buffer_escrituras = 256*24, offset = 0; 
        struct REGISTRO buffer_escrituras[cant_registros_buffer_escrituras];
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));
        while (mi_read(rutaPrueba, buffer_escrituras, offset, sizeof(buffer_escrituras)) > 0)
        {
            // leer una escritura
            for (int i = 0; i < cant_registros_buffer_escrituras; i++)
            {
                // la escritura e valida?
                if (buffer_escrituras[i].pid == info.pid){ // para no leer basura
                        
                    // es la primera escritura valida?
                    if (info.nEscrituras == 0)
                    {
                        // Inicializar los registros significativos con los datos de esa escritura
                        info.MayorPosicion = buffer_escrituras[i];
                        info.UltimaEscritura = buffer_escrituras[i];
                        info.MenorPosicion = buffer_escrituras[i];
                        info.PrimeraEscritura = buffer_escrituras[i];
                        // ya será la de menor posición puesto que hacemos un barrido secuencial
                    }
                    else
                    {
                        // Comparar nº de escritura (para obtener primera y última) y actualizarlas si es preciso
                    if (buffer_escrituras[i].nEscritura > info.UltimaEscritura.nEscritura){//Última escritura: registro con el nº de escritura mayor
                        info.UltimaEscritura = buffer_escrituras[i];
                        }
                    if (buffer_escrituras[i].nEscritura < info.PrimeraEscritura.nEscritura){//Primera escritura: registro con el nº de escritura menor
                        info.PrimeraEscritura = buffer_escrituras[i];
                        }
                        if (buffer_escrituras[i].nRegistro < info.MenorPosicion.nRegistro){//Menor posición: registro con la posición (nº registro) más baja
                        info.MenorPosicion = buffer_escrituras[i];
                        }
                    if (buffer_escrituras[i].nRegistro > info.MayorPosicion.nRegistro){//Mayor posición: registro con la posición (nº registro) más alta
                        info.MayorPosicion = buffer_escrituras[i];
                        }
                        }
                         // incrementar nº escrituras validas
                info.nEscrituras++;
                }
               
            }
            memset(&buffer_escrituras, 0, sizeof(buffer_escrituras));
            offset += sizeof(buffer_escrituras); // actualizar el offset en cada iteración
        }

  // guardar la informacion de las escituras en el fichero

        char tiempoPrimero[80], tiempoUltimo[80],  tiempoMenor[80], tiempoMayor[80];

        strcpy(tiempoPrimero, formatearTiempo(info.PrimeraEscritura.fecha));
        strcpy(tiempoUltimo, formatearTiempo(info.UltimaEscritura.fecha));
        strcpy(tiempoMenor, formatearTiempo(info.MenorPosicion.fecha));
        strcpy(tiempoMayor, formatearTiempo(info.MayorPosicion.fecha));

        char buffer[BLOCKSIZE];
        memset(buffer, 0, BLOCKSIZE);

        snprintf(buffer, BLOCKSIZE, 
            "PID: %d\nNumero de escrituras:\t%d\nPrimera escritura:"
                "\t%d\t%d\t%s\nUltima escritura:\t%d\t%d\t%s\nMayor po"
                "sición:\t\t%d\t%d\t%s\nMenor posición:\t\t%d\t%d\t%s\n\n",
                info.pid, info.nEscrituras,
                info.PrimeraEscritura.nEscritura,
                info.PrimeraEscritura.nRegistro,
                tiempoPrimero,
                info.UltimaEscritura.nEscritura,
                info.UltimaEscritura.nRegistro,
                tiempoUltimo,
                info.MenorPosicion.nEscritura,
                info.MenorPosicion.nRegistro,
                tiempoMenor,
                info.MayorPosicion.nEscritura,
                info.MayorPosicion.nRegistro,
                tiempoMayor);

        fprintf(stderr, GRAY "[%i) %i escrituras validadas en %s]\n" RESET, i + 1, info.nEscrituras, informePath);

    


    //  Añadir la información del struct info al fichero buffer_informe.txt por el final.
     bytes_escritos += mi_write(informePath, &buffer, bytes_escritos, strlen(buffer));
    if (bytes_escritos < 0)
    {
       // mostrar_error_buscar_entrada(error);
        return FALLO;
    } 
    }


// Desmontar el dispositivo virtual
if (bumount() == FALLO)
{
    fprintf(stderr, RED "verificacion.c: Error al desmontar el dispositivo.\n" RESET);
    return FALLO;
}

return EXITO;
}
