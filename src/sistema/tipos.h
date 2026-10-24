/*---------------------------------------------------------------------------------
    tipos.h — Tipos enteros portables: los mismos nombres en la SNES y en el PC.

    PROPÓSITO
      La lógica pura del juego (tarou.c, camara.c, nivel.c, animaciones.c) se
      compila dos veces: con tcc816 para la ROM y con gcc para las pruebas de
      tests/. En la SNES, u8/u16/s16 los define <snes.h>; en el PC no existe
      snes.h, así que los tomamos de <stdint.h>. Quien incluye este archivo no
      tiene que saber en cuál de los dos mundos está.

    SE CONECTA CON
      Incluye:  <snes.h> (ROM) o <stdint.h> (PC, cuando está definido PRUEBAS_PC;
                lo define tests/Makefile con -DPRUEBAS_PC).
      Lo usan:  sistema/puntofijo.h, las cabeceras de lógica pura de juego/ y
                tests/minitest.h.

    HARDWARE QUE TOCA
      ninguno. Regla de la CPU que motiva estos tipos: el 65816 es de 16 bits,
      así que u16/s16 son los enteros "naturales" y rápidos; int de 32 bits o
      float serían lentos o inexistentes.

    VARIABLES (tipos que define)
      u8   entero sin signo de 8 bits    0..255
      u16  entero sin signo de 16 bits   0..65535
      s8   entero con signo de 8 bits    -128..127
      s16  entero con signo de 16 bits   -32768..32767 (velocidades, posiciones en punto fijo)

    → glosario: u8/u16, s16, #ifndef
---------------------------------------------------------------------------------*/
#ifndef TIPOS_H
#define TIPOS_H

#ifdef PRUEBAS_PC
#include <stdint.h>          /* Enteros de tamaño fijo del C estándar.                 */
typedef uint8_t u8;          /* "typedef" da un nombre nuevo a un tipo que ya existe.  */
typedef uint16_t u16;
typedef int8_t s8;
typedef int16_t s16;
#else
#include <snes.h>            /* PVSnesLib ya define u8, u16, s8, s16.                  */
#endif

#endif /* TIPOS_H */
