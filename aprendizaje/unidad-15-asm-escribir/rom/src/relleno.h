/*---------------------------------------------------------------------------------
    relleno.h — Cómo ve C las rutinas de relleno de la mini-ROM de la unidad 15.

    PROPÓSITO
      Declarar, con la sintaxis de C de siempre, dos rutinas que hacen LO MISMO
      (escribir `cuantas` veces la misma palabra en la VRAM): una escrita en
      ensamblador (relleno.asm) y otra en C (relleno_c.c). Para C las dos son
      funciones normales: no sabe ni le importa en qué lenguaje están escritas;
      el enlazador (wlalink) une cada llamada con la etiqueta del mismo nombre.
      También declara las dos maneras de vaciar un búfer de la WRAM que se
      comparan en el ejercicio (un bucle en C y el memset de la librería).

    SE CONECTA CON
      Incluye:  <snes.h> (u8, u16).
      Implementa: relleno.asm (relleno_vram, relleno_llamadas) y relleno_c.c
                (relleno_vram_c, limpiar_c).
      Lo usa:   principal.c.

    HARDWARE QUE TOCA
      ninguno aquí (las rutinas: INIDISP $2100, VMAIN $2115, VMADD $2116, VMDATA $2118).

    VARIABLES
      relleno_llamadas  u16  veces que se ha llamado a relleno_vram (vive en relleno.asm)

    → glosario: VRAM, Tilemap, Forced blank
---------------------------------------------------------------------------------*/
#ifndef RELLENO_H
#define RELLENO_H

#include <snes.h>

/* Definida en relleno.asm, dentro de una RAMSECTION: para C es una variable más. */
extern u16 relleno_llamadas;

/* Escribe `cuantas` veces `palabra` en la VRAM a partir de la dirección `vram` (en
   palabras). EN ENSAMBLADOR (relleno.asm). Apaga la pantalla mientras escribe
   (forced blank) y la deja como la tenía el SDK (mirrorINIDISP). */
void relleno_vram(u16 vram, u16 palabra, u16 cuantas);

/* Lo mismo, escrito en C con las macros REG_xxx de la librería (relleno_c.c). */
void relleno_vram_c(u16 vram, u16 palabra, u16 cuantas);

/* Pone a 0 `n` bytes a partir de `p` con un bucle `*p++ = 0` en C (relleno_c.c),
   para comparar su ensamblador con el de memset. */
void limpiar_c(u8 *p, u16 n);

#endif /* RELLENO_H */
