#include "ficheros_basico.h"
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo) {
	#define NPUNTEROS (BLOCKSIZE / sizeof(unsigned int)) // 256
	#define NPUNTEROS2 (NPUNTEROS * NPUNTEROS)           // 65.536
	#define NPUNTEROS3 (NPUNTEROS * NPUNTEROS * NPUNTEROS) // 16.777.216
	unsigned int nivel_punteros, nblog, ultimoBL;
	unsigned char bufAux_punteros[BLOCKSIZE];
	unsigned int bloques_punteros[3][NPUNTEROS];
	int indices_primerBL[3];   // indices del primerBL para cuando se llama desde mi_truncar_f()
	int liberados = 0;
	int i, j, k; //para iterar en cada nivel de punteros
	int eof = 0; //para determinar si hemos llegado al último BL
	int contador_breads = 0;  //para comprobar optimización eficiencia
	int contador_bwrites = 0; //para comprobar optimización eficiencia
	int bloque_modificado[3] = {0,0,0}; //para saber si se ha modificado un bloque de punteros de algún nivel
	#if DEBUG
	   int BLliberado = 0;  //utilizado para imprimir el nº de bloque lógico que se ha liberado 
	#endif
 
 
	if (inodo->tamEnBytesLog == 0)
		return 0;
 
 
	if (inodo->tamEnBytesLog % BLOCKSIZE == 0) {
		ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE - 1;
	} else {
		ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
	}
 
 
	#if DEBUG
	fprintf(stderr, "[liberar_bloques_inodo()→ primer BL: %d, último BL: %d]\n", primerBL, ultimoBL);
	#endif
 
 
	memset(bufAux_punteros, 0, BLOCKSIZE);
 
 
	//liberamos los bloques de datos de punteros directos
	if (primerBL<DIRECTOS){
		nivel_punteros=0;
		i=obtener_indice(primerBL,nivel_punteros);
		while (!eof  && i<DIRECTOS){
			nblog=i;
			if (nblog==ultimoBL) eof=1;
			if (inodo->punterosDirectos[i]){
				liberar_bloque(inodo->punterosDirectos[i]);
				#if DEBUG
				fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]\n", inodo->punterosDirectos[i], nblog);
				//BLliberado=nblog;
				#endif
				liberados++;
				inodo->punterosDirectos[i]=0;
			}
			i++;
		}
	}
	//liberamos los bloques de datos e índice de Indirectos[0]
	if (primerBL<INDIRECTOS0 && !eof){
		nivel_punteros=1;
		if (inodo->punterosIndirectos[0]) {
			bread(inodo->punterosIndirectos[0], bloques_punteros[nivel_punteros-1]);
			bloque_modificado[nivel_punteros-1] = 0;
			contador_breads++;
			if (primerBL >= DIRECTOS){
				i=obtener_indice(primerBL,nivel_punteros);
			}else {
				i=0;
			}
			while (!eof && i<NPUNTEROS){
				nblog=DIRECTOS+i;
				if (nblog==ultimoBL) eof=1;
				if (bloques_punteros[nivel_punteros-1][i]){
					liberar_bloque(bloques_punteros[nivel_punteros-1][i]);
					#if DEBUG
					fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]\n", bloques_punteros[nivel_punteros-1][i], nblog);
					BLliberado=nblog;
					#endif
					liberados++;
					bloques_punteros[nivel_punteros-1][i]=0;
					bloque_modificado[nivel_punteros-1]=1;
				}
				i++;
			}
			if (memcmp(bloques_punteros[nivel_punteros-1], bufAux_punteros, BLOCKSIZE) == 0) {
				liberar_bloque(inodo->punterosIndirectos[0]); //de punteros
				#if DEBUG
				fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n", inodo->punterosIndirectos[0], nivel_punteros, BLliberado);
				#endif
				liberados++;
				inodo->punterosIndirectos[0]=0;
			} else { // escribimos en el dispositivo el bloque de punteros, si ha sido modificado
				if (bloque_modificado[nivel_punteros-1]) {
					if (bwrite(inodo->punterosIndirectos[0], bloques_punteros[nivel_punteros-1]) < 0) return -1;
					contador_bwrites++;
				}
			}  
		}
	}
	//liberamos los bloques de datos e índice de Indirectos[1]
	if (primerBL<INDIRECTOS1 && !eof){
		nivel_punteros=2;
		indices_primerBL[0]=0;
		indices_primerBL[1]=0;
		if (inodo->punterosIndirectos[1]) {
			bread(inodo->punterosIndirectos[1], bloques_punteros[nivel_punteros-1]);
			bloque_modificado[nivel_punteros-1]=0;
			contador_breads++;
			if (primerBL >= INDIRECTOS0){
				i=obtener_indice(primerBL,nivel_punteros); 
			} else i=0;
			indices_primerBL[nivel_punteros-1]=i;
			while (!eof && i<NPUNTEROS){
				if (bloques_punteros[nivel_punteros-1][i]){
					bread(bloques_punteros[nivel_punteros-1][i], bloques_punteros[nivel_punteros-2]);
					bloque_modificado[nivel_punteros-2] = 0;
					contador_breads++;
					if (i== indices_primerBL[nivel_punteros-1]) {
						j=obtener_indice(primerBL,nivel_punteros-1);
						indices_primerBL[nivel_punteros-2]=j;
					} else j=0;
				   
					while (!eof && j<NPUNTEROS){
						nblog=INDIRECTOS0+i*NPUNTEROS+j;
						if (nblog==ultimoBL) eof=1;
						if (bloques_punteros[nivel_punteros-2][j]){
							liberar_bloque(bloques_punteros[nivel_punteros-2][j]);
							#if DEBUG
							fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]\n", bloques_punteros[nivel_punteros-2][j], nblog);
							BLliberado=nblog;
							#endif
							liberados++;
							bloques_punteros[nivel_punteros-2][j]=0;
							bloque_modificado[nivel_punteros-2]=1;
						}
						j++;
					}
					if (memcmp(bloques_punteros[nivel_punteros-2], bufAux_punteros, BLOCKSIZE) == 0) {
						liberar_bloque(bloques_punteros[nivel_punteros-1][i]);//de punteros
						#if DEBUG
						fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n", bloques_punteros[nivel_punteros-1][i], nivel_punteros-1, BLliberado);
						#endif
						liberados++;
						bloques_punteros[nivel_punteros-1][i]=0;
						bloque_modificado[nivel_punteros-1]=1;
					} else { // escribimos en el dispositivo el bloque de punteros, si ha sido modificado
						if (bloque_modificado[nivel_punteros-2]) {
							if (bwrite(bloques_punteros[nivel_punteros-1][i], bloques_punteros[nivel_punteros-2]) < 0) return -1;
							contador_bwrites++;
						}
					} 
				}
				i++;
			}
			if (memcmp(bloques_punteros[nivel_punteros-1], bufAux_punteros, BLOCKSIZE) == 0) {
				liberar_bloque(inodo->punterosIndirectos[1]);    //de punteros
				#if DEBUG
				fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n",inodo->punterosIndirectos[1], nivel_punteros, BLliberado);
				#endif
				liberados++;
				inodo->punterosIndirectos[1]=0;
			} else { // escribimos en el dispositivo el bloque de punteros, si ha sido modificado
				if (bloque_modificado[nivel_punteros-1]) {
					if (bwrite(inodo->punterosIndirectos[1], bloques_punteros[nivel_punteros-1]) < 0) return -1;
					contador_bwrites++;
				}
			} 
		}
	}
 
 
	//liberamos los bloques de datos e índice de Indirectos[2]
	if (primerBL<INDIRECTOS2 && !eof){
		nivel_punteros=3;
		indices_primerBL[0]=0;
		indices_primerBL[1]=0;
		indices_primerBL[2]=0;
		if (inodo->punterosIndirectos[2]) {
			bread(inodo->punterosIndirectos[2], bloques_punteros[nivel_punteros-1]);
			bloque_modificado[nivel_punteros-1]=0;
			contador_breads++;
			if (primerBL >= INDIRECTOS1){
				i=obtener_indice(primerBL,nivel_punteros);
				indices_primerBL[nivel_punteros-1]=i;
			} else i=0;
			while (!eof && i<NPUNTEROS){
				if (bloques_punteros[nivel_punteros-1][i]){
					bread(bloques_punteros[nivel_punteros-1][i], bloques_punteros[nivel_punteros-2]);
					contador_breads++;
					if (i== indices_primerBL[nivel_punteros-1]) {
						j=obtener_indice(primerBL,nivel_punteros-1);
						indices_primerBL[nivel_punteros-2]=j;
					} else j=0;
					while (!eof && j<NPUNTEROS){
						if (bloques_punteros[nivel_punteros-2][j]){
							bread(bloques_punteros[nivel_punteros-2][j], bloques_punteros[nivel_punteros-3]);
							contador_breads++;
							if (i== indices_primerBL[nivel_punteros-1] && j==indices_primerBL[nivel_punteros-2]) {
								k=obtener_indice(primerBL,nivel_punteros-2);
								indices_primerBL[nivel_punteros-3]=k;
							} else k=0;
							while (!eof && k<NPUNTEROS){
								nblog=INDIRECTOS1+i*NPUNTEROS2+j*NPUNTEROS+k;
								if (nblog==ultimoBL) eof=1;
								if (bloques_punteros[nivel_punteros-3][k]){
									liberar_bloque(bloques_punteros[nivel_punteros-3][k]);
									#if DEBUG
									fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]\n", bloques_punteros[nivel_punteros-3][k], nblog);
									BLliberado=nblog;
									#endif
									liberados++;
									bloques_punteros[nivel_punteros-3][k]=0;
									bloque_modificado[nivel_punteros-3]=1;
								}
								k++;
							}
							if (memcmp(bloques_punteros[nivel_punteros-3], bufAux_punteros, BLOCKSIZE) == 0) {
								liberar_bloque(bloques_punteros[nivel_punteros-2][j]);//de punteros
								#if DEBUG
								fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n",bloques_punteros[nivel_punteros-2][j], nivel_punteros-2, BLliberado);
								#endif
								liberados++;
								bloques_punteros[nivel_punteros-2][j]=0;
								bloque_modificado[nivel_punteros-2]=1;
							} else { // escribimos en el dispositivo el bloque de punteros, si ha sido modificado
								if (bloque_modificado[nivel_punteros-3]) {
									if (bwrite(bloques_punteros[nivel_punteros-2][j], bloques_punteros[nivel_punteros-3]) < 0) return -1;
									contador_bwrites++;
								}
							}   
						}
						j++;
					}
					if (memcmp(bloques_punteros[nivel_punteros-2], bufAux_punteros, BLOCKSIZE) == 0) {
						liberar_bloque(bloques_punteros[nivel_punteros-1][i]);//de punteros
						#if DEBUG
						fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n",bloques_punteros[nivel_punteros-1][i], nivel_punteros-1, BLliberado);
						#endif
						liberados++;
						bloques_punteros[nivel_punteros-1][i]=0;
						bloque_modificado[nivel_punteros-1]=1;
					} else { // escribimos en el dispositivo el bloque de punteros, si ha sido modificado
						if (bloque_modificado[nivel_punteros-2]) {
							if (bwrite(bloques_punteros[nivel_punteros-1][i], bloques_punteros[nivel_punteros-2]) < 0) return -1;
							contador_bwrites++;
						}
					}  
				}
				i++;
			}
			if (memcmp(bloques_punteros[nivel_punteros-1], bufAux_punteros, BLOCKSIZE) == 0) {
				liberar_bloque(inodo->punterosIndirectos[2]);//de punteros
				#if DEBUG
				fprintf(stderr, "[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n",inodo->punterosIndirectos[2], nivel_punteros, BLliberado);
				#endif
				liberados++;
				inodo->punterosIndirectos[2]=0;       
			} else { // escribimos en el dispositivo el bloque de punteros, si ha sido modificado
				if (bloque_modificado[nivel_punteros-1]) {
					if (bwrite(inodo->punterosIndirectos[2], bloques_punteros[nivel_punteros-1]) < 0) return -1;
					contador_bwrites++;
				}
			}      
		}
	}
  
	#if DEBUG
	fprintf(stderr,"[liberar_bloques_inodo()→ total bloques liberados: %d, total_breads: %d, total_bwrites:%d]\n", liberados, contador_breads, contador_bwrites);
	#endif
	return liberados;
 