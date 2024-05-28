/*
Autores: Khaoula Ikkene, Tomás Gallardo Rago, Francesc Gayá Piña  
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

 
En cuanto a las mejoras nuevas, se ha implementado la caché con tabla FIFO por Khaoula Ikkene y también LRU (aunque esta por el momento no funciona del todo).
Restricciones: ninguna.



