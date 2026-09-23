/*---------------------------------------------------------------------------------
    comun.h — Utilidades que comparten todas las pantallas: borrar la pantalla de
    texto, centrar texto, efecto máquina de escribir y fundidos.
---------------------------------------------------------------------------------*/
#ifndef COMUN_H
#define COMUN_H

#include <snes.h>

/* Cuenta las letras de un texto (sin contar el 0 final que cierra toda cadena en C). */
u8 longitud(char *texto);

/* Borra las 28 filas de la capa de texto escribiendo espacios. */
void limpiar_texto(void);

/* Escribe `texto` centrado horizontalmente en la fila `fila` (0..27). */
void texto_centrado(u8 fila, char *texto);

/* Máquina de escribir: escribe centrado en `fila` solo las primeras `letras`
   letras de `texto`. Devuelve cuántas letras "sobraron" para la siguiente
   línea (0 si el texto aún no se completó). Así se encadenan varias líneas. */
u16 escribir_parcial(u8 fila, char *texto, u16 letras);

/* Fundidos. Bloquean el programa mientras duran (lo hace la librería). */
void fade_in(u8 frames);
void fade_out(u8 frames);

#endif /* COMUN_H */
