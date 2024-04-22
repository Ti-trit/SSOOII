#include "directorios.h"

#define TAMFILA 100
#define TAMBUFFER (TAMFILA*1000) //suponemos un máx de 1000 entradas, aunque debería ser SB.totInodos
/**
 * Programa que lista el contenido de una directior/fichero
*/
int main (int argc, char *argv[]){

    
    //comporbar sintaxis
    if (argc<3){
        fprintf(stderr, RED "Sintaxis:./mi_ls <disco> </ruta> o ./mi_ls -l <disco> </ruta>.\n " RESET);
        return FALLO;
    }

    //montar el dispositivo
    if (bmount(argv[1]<0)){
        fprintf(stderr, RED "mi_ls.c: Error al montar el dispositivo\n"RESET);
        return FALLO;
    }

    const char *camino;
    char ruta;    
    int formato = 0; //simple
    if (strcmp(argv[1], "-l")==0){
        camino = argv[2];
        ruta = argv[3];
        formato= 1;//formato extendido
    }else{
        camino = argv[1];
        ruta = argv[2];
    }
    
    //determinar el tipo
       char tipo = (camino[strlen(camino) - 1] != '/') ? 'f' : 'd';


    char buffer[TAMBUFFER];
    //llamamos a la funcion mi_dir

    int nEntradas = mi_dir(argv[1], buffer,tipo);
    if (nEntradas<0){
        mostrar_error_buscar_entrada(nEntradas);
        return FALLO;
    }
    if (tipo == 'd'){
        printf("Total: %i", nEntradas);
    }
   fprintf(stdout, "Tipo\tModo\tmTime\t\t\tTamaño\tNombre\n--------------------------------------------------------------------------------\n%s\n", buffer);

if (formato == 1){       
 char *info = strtok(buffer,"\n");
 
    for (int i =0; i<nEntradas; i++){
        info = strtok(NULL, "\t");
        fprintf(stdout,"%s", info );
    }
    
}
    
    //desmontar el dispositivo
    if (bumount()==FALLO){
        fprintf(stderr, RED "mi_ls.c: Error al desmontar el dispositivo\n"RESET);
        return FALLO;
    }

return EXITO;

}
























