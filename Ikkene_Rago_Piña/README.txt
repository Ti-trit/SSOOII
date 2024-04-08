/*
Autores: Khaoula Ikkene, Tomás Gallardo Rago, Francesc Gayá Piña  
Grupo: AntiLinux
*/

Para la sintaxis específica hemos seguido el formato recomendado en las instrucciones
de los diferentes niveles. 
Así que la sintaxis de los programas ficticios y mi_mkfs.c quedan así: 
        escribir.c: escribir <nombre_dispositivo> <'$(cat fichero)'> <diferentes_inodos>
        leer_sf:  <nombre_dispositivo>
        leer.c: <nombre_dispositivo> <ninodo>
        permitir.c : permitir <nombre_dispositivo> <ninodo> <permisos>
        truncar.c : truncar <nombre_dispositivo> <ninodo> <nbytes>

En cuanto a las mejoras no se ha realizado ninguna, y en el método liberar_bloque_inodo() las mejoras que hemos introducido (saltar los bloques que no hace falta explorar) no optimizan
, como se espera, la cantidad de breads y bwrites. 
Y finalmente no hay ninguna restricción.




