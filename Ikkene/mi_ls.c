/**
* @authors Khaoula Ikkene
**/
#include "directorios.h"
#define TAMFILA 100
#define TAMBUFFER (TAMFILA*1000) //suponemos un máx de 1000 entradas, aunque debería ser SB.totInodos

/**
 * Programa que lista el contenido de una directior/fichero
 * Se ha añadido el parametro formato. 0 para formato simple, y 1 para formato expandido
 * Se puede usar también para los ficheros.
*/
int main(int argc, char const *argv[]){

    if(argc != 4){
        fprintf(stderr, RED "Error de sintaxis. Uso correcto: ./mi_ls <disco> </ruta_directorio> <formato>\n" RESET);
        return FALLO;
    }

    // Obtener argumentos
    char *camino = (char*)argv[2];
    int formato = atoi(argv[3]); 
    
    // Montaje del dispositivo virtual
    if (bmount(argv[1]) == FALLO) {
        fprintf(stderr, RED "Error al montaje el dispositivo virtual\n" RESET);
        return FALLO;
    }


char buff[TAMBUFFER];
memset(buff, 0, TAMBUFFER);

char tipo;
if (camino[strlen(camino)-1] != '/') {
    tipo = 'f';
} else {
    tipo = 'd';
}

if (tipo == 'f') {
    int index = strlen(camino) - 1;
    while (index >= 0) {
        if (camino[index] == '/') {
            break;
        }
        index--;
    }

    char *nuevoCamino = malloc(index + 1); // Asignar memoria para nuevoCamino
    strncpy(nuevoCamino, camino, index+1);
    nuevoCamino[index+1] = '\0'; // Marcar fin de cadena con '\0'

    char *nombreFichero = malloc(strlen(camino) - index); // Asignar memoria para nombreFichero
    strncpy(nombreFichero, camino + index + 1, strlen(camino) - index);
    nombreFichero[strlen(camino) - index] = '\0'; 
    int numEntradas = mi_dir(nuevoCamino, buff, 'd');
  
    char *linea = strtok(buff, "\n"); // Obtener la primera línea del buffer
    for (int i = 0; i < numEntradas; i++) {
        if (strstr(linea, nombreFichero) != NULL) {
            fprintf(stdout, "Tipo\tModo\tmTime\t\t\tTamaño\tNombre\n--------------------------------------------------------------------------------\n%s\n", linea);
            break;
        }
        linea = strtok(NULL, "\n"); // Avanzar a la siguiente línea
    }
// Liberar la memoria asignada
    free(nuevoCamino); 
    free(nombreFichero);
}else{


    memset(buff, 0, TAMBUFFER);
   
    int numEntradas = mi_dir(argv[2], buff, tipo);
     if (tipo == 'd' && numEntradas>-1) {
        fprintf(stdout, "Total: %d\n",numEntradas);
    }
    if (numEntradas < 0) {
        mostrar_error_buscar_entrada(numEntradas);
        return FALLO;
    } else if (numEntradas > 0) {
    if (formato == 0) {//formato simple
    char *linea = strtok(buff, "\n"); // Obtener la primera línea del buffer
    for (int i = 0; i < numEntradas && linea != NULL; i++) {
        char *ultimoTab = strrchr(linea, '\t'); // Encontrar el último tabulador
       if (ultimoTab != NULL) {
            fprintf(stdout, "%.*s", 5, linea); // Imprimir directamente el color (los primeros 5 caracteres)
            fprintf(stdout, "%s\t", ultimoTab + 1); // Imprimir el nombre de la entrada
        }
        linea = strtok(NULL, "\n"); // Avanzar a la siguiente línea
    }
            fprintf(stdout, RESET "\n");
        } else {//formato expandido
           
            fprintf(stdout, "Tipo\tModo\tmTime\t\t\tTamaño\tNombre\n--------------------------------------------------------------------------------\n%s\n", buff);
        }
    }
}
    
    if (bumount() == FALLO){
        fprintf(stderr, RED "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    return EXITO;
}