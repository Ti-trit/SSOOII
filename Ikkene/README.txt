/**
@author Khaoula Ikkene
Grupo: AntiLinux
*/

Para la sintaxis específica hemos seguido el formato recomendado en las instrucciones
de los diferentes niveles. 
Así que la sintaxis de los programas ficticios y mi_mkfs.c quedan así: 
        escribir.c: ./escribir <nombre_dispositivo> <'$(cat fichero)'> <diferentes_inodos>
        leer_sf:  ./leer_sf <nombre_dispositivo>
        leer.c: ./leer.c <nombre_dispositivo> <ninodo>
        permitir.c : ./permitir <nombre_dispositivo> <ninodo> <permisos>
        truncar.c : ./truncar <nombre_dispositivo> <ninodo> <nbytes>
        mi_cat.c : ./mi_cat <disco> </ruta_fichero>
        mi_chmod.c : ./mi_chmod <nombre_dispositivo> <permisos> </ruta>
        mi_link.c : ./mi_link disco /ruta_fichero_original /ruta_enlace
        mi_ls.c : ./mi_ls <disco> </ruta_directorio> <formato>
        mi_mkdir.c : ./mi_mkdir <nombre_dispositivo> <permisos> </ruta_directorio/>
        mi_mkfs.c : ./mi_mkfs <nombre_dispositivo> <nbloques>
        mi_rm_r.c : ./mi_rm_r disco /ruta
        mi_rm.c : ./mi_rm disco /ruta
        mi_rmdir.c : ./mi_rm disco /ruta
        mi_stat.c : ./mi_stat <disco> </ruta>
        mi_touch.c : <mi_touch> <nombre_dispositivo> <permisos> </ruta>
        mi_escribir.c: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>
        mi_rn.c: ./mi_rn <disco> </ruta/antiguo> <nuevo> 
        mi_cp_f.c: ./mi_cp_f <disco> </origen/nombre> </destino/>
        mi_cp.c: ./mi_cp <disco> </origen/nombre> </destino/>
        mi_mv.c:./mi_mv <disco> </origen/nombre> </destino/>
        verificacion.c: ./verificacion <nombre_dispositivo> <directorio_simulación>
        simulacion.c: ./simulacion <disco>


Mejoras realizadas ultimamente: caché de directorios, las funcionalidades extra(mi_rn, mi_cp_f, mi_cp, mi_mv), y mmap.
En cuanto a la optimización que hice del metodo liberar_bloques_inodo no pude solucionar los errores que causaba a la hora de liberar algunos bloques.
Por ello probé de hacer las dos mejoras de la versión iterativa compacta, y parece que tengo parte de la primera mejora. 
Y probando con la compactación de la versión recursiva, he combinado en una sola función recrusiva la liberación de los bloques directos
y indirectos(liberar_bloques_auxiliar). 




