/**
Autores: Khaoula Ikkene
**/

#include "bloques.h"
#include "semaforo_mutex_posix.h"
//Variables globales
static sem_t *mutex;
static unsigned int inside_sc = 0;
static int descriptor = 0;
static int tamSFM; //tamaño memoria compartida
#if DEBUGMMAP
static void *ptrSFM;//puntero a memoria compartida
#endif


/**
 * Monta el dispositivo virtual, abre el fichero
 * @param camino      nombre del fichero
 * @return           el descriptor o -1 en caso de error
 * 
*/
int bmount(const char *camino){

 if (descriptor > 0) {
        close(descriptor);
    }
       if (!mutex) { // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
       mutex = initSem(); 
       if (mutex == SEM_FAILED) {
           return FALLO;
       }
   }

    // Configurar umask del proceso para no restringir permisos
    umask(000);
    
    // abrir el archivo con permisos de lectura y escritura,
    // creandolo si no existe
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);

    //llamar a mmap
    #if DEBUGMMAP
    ptrSFM= do_mmap(descriptor);
    #endif    

    //Verifica si hubo un error al abrir el archivo
    if (descriptor == -1) {
        fprintf(stderr, RED "Error al abrir el fichero.\n");
        return FALLO;
    }
    //Si todo va bien, retorn el descriptor del archivos
    return descriptor;
}

/**
 * Desmonta el dispositivo virtual
 * @return  0 si lo ha cerrado correctamente. -1 en otro caso
*/
int bumount() {
    #if DEBUGMMAP
    //volcar las paginas a disco con mysunc
    
    if (msync(ptrSFM, tamSFM, MS_SYNC)<0){
        fprintf(stderr, RED "bumount: Error al volcar las paginas del disco\n"RESET);
        return FALLO;
    }
    
    //liberar la memoria ocupada por el fichero
    if (munmap(ptrSFM, tamSFM)<0){
        fprintf(stderr, RED "bumount: Error al liberar al memoria compartida\n "RESET);
        return FALLO;
    }

    #endif

    descriptor = close(descriptor);
     deleteSem();
    //Si no se ha cerrado devuelve -1
    if (close(descriptor) == -1) {
        return FALLO;
    }
    
    //Si se ha cerrado devuelve 0
    return EXITO;
}

/**
 * Escribe 1 bloque en el dispositivo virtual, en el bloque físico (nbloque).
 * @param nbloque     posición virtual del bloque
 * @param buf         buffer de memoria para escribir 
 * @return            número de bytes escritos 
*/
int bwrite(unsigned int nbloque, const void *buf){
int pos = nbloque*BLOCKSIZE;//poisicon del bloque
#if !DEBUGMMAP
if (lseek(descriptor, pos, SEEK_SET)<0){   //posicionar el puntero dentro del bloque
    fprintf(stderr, RED NEGRITA"Error al posicionar el puntero en bwrite().\n"RESET);
    return FALLO;
}
//Escribir en el bloque
int nbytes = write(descriptor,buf,BLOCKSIZE);
if (nbytes<0){
    fprintf(stderr,RED"Error al escribir el bloque %i.\n" RESET, nbloque);
        return FALLO;
}else{
    //número de bytes escritos en el fichero
    return nbytes;
}

#else
int nbytes;
if (nbloque*BLOCKSIZE + BLOCKSIZE<=tamSFM){
    nbytes = BLOCKSIZE;
}else{
    nbytes = tamSFM-(nbloque*BLOCKSIZE);
}
if (nbytes>0){
   memcpy(ptrSFM + pos, buf, nbytes);
}
return nbytes;



#endif

}

/**
 * Lee 1 bloque del dispositivo virtual, correspondiente al físico nbloque.
 * @param nbloque     posición virtual del bloque
 * @param buf         buffer de memoria cuyo contenido intentaremos leer
 * @return             número de bytes leedos.FALLO en caso de error 
*/
int bread(unsigned int nbloque, void*buf){
    int pos = nbloque*BLOCKSIZE;// Posición inicial
        //posicionar el puntero
    #if !DEBUGMMAP    
    if (lseek(descriptor, pos, SEEK_SET)<0){
        fprintf(stderr, RED NEGRITA"Error al posicionar el puntero en bread().\n"RESET);
        return FALLO; // Error al posicionar el puntero
    }

    // Lee el contenido del bloque
    int nbytes = read(descriptor,buf,BLOCKSIZE);
    #else
    int nbytes;
    if ((nbloque * BLOCKSIZE) + BLOCKSIZE<=tamSFM){
        nbytes = BLOCKSIZE;
    }else{
        nbytes = tamSFM-(BLOCKSIZE*nbloque);
    }
    if (nbytes>0){
    memcpy(buf, ptrSFM + pos, nbytes);

    }

    #endif


    if(nbytes>=0){
        return nbytes; // Devuelve nbytes == BLOCKSIZE
    }else{
        fprintf(stderr, RED NEGRITA"bread: Error al leer en el bloque %i.\n"RESET, nbloque);
        return FALLO; // Salta un error en el bloque "nbloque"
    }
}


void mi_waitSem() {
   if (!inside_sc) { // inside_sc==0, no se ha hecho ya un wait
       waitSem(mutex);
   }
   inside_sc++;
}


void mi_signalSem() {
   inside_sc--;
   if (!inside_sc) {
       signalSem(mutex);
   }
}


void *do_mmap(int fd) {
   struct stat st;
   void *ptr;
   fstat(fd, &st);
   tamSFM = st.st_size; //static int tamSFM: tamaño memoria compartida
   if ((ptr = mmap(NULL, tamSFM, PROT_WRITE, MAP_SHARED, fd, 0))== (void *)-1)
       fprintf(stderr, "Error %d: %s\n", errno, strerror(errno)); 
   return ptr;
}
