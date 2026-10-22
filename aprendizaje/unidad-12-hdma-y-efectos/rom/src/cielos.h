/*---------------------------------------------------------------------------------
    cielos.h — Las tablas HDMA de color de la mini-ROM de la unidad 12.

    PROPÓSITO
      Declara las dos tablas que efectos12.c le pasa a setModeHdmaColor: el
      cielo de día (hecha) y el de atardecer (el ejercicio). Define también
      FRANJA, la macro que escribe UNA entrada de la tabla (5 bytes).

    SE CONECTA CON
      Incluye:  <snes.h> (u8, RGB5).
      Lo usan:  src/cielos.c (define las tablas), src/efectos12.c (las usa).

    HARDWARE QUE TOCA
      ninguno: son datos. Los lee el canal 6 de HDMA (ver efectos12.c).

    VARIABLES (globales)
      cielo_dia        const u8[]  28 franjas de 8 líneas, de azul oscuro a claro
      cielo_atardecer  const u8[]  el ejercicio (TODO en cielos.c)
    → glosario: DMA / HDMA, CGRAM, Paleta
---------------------------------------------------------------------------------*/
#ifndef CIELOS_H
#define CIELOS_H

#include <snes.h>

/* Una entrada de la tabla que espera setModeHdmaColor (modo 3 de HDMA, destino
   $2121): primero CUÁNTAS líneas dura (1..127; el bit 7 a 1 significaría «modo
   repetir»), y después los 4 bytes que el canal escribe en
   $2121, $2121, $2122, $2122:
     0x00, 0x00  CGADD = 0 dos veces: el color que se cambia es el 0 de la CGRAM
                 (el color de fondo, el que se ve donde todas las capas son
                 transparentes);
     lo, hi      CGDATA: el color de 15 bits, byte bajo y byte alto.
   r, g y b van de 0 a 31 (5 bits cada uno, RGB5 de video.h). */
#define FRANJA(lineas, r, g, b) \
    (lineas), 0x00, 0x00, (u8)(RGB5(r, g, b) & 0xFF), (u8)(RGB5(r, g, b) >> 8)

extern const u8 cielo_dia[];         /* La tabla del cielo de día (termina en 0).         */
extern const u8 cielo_atardecer[];   /* La tabla del atardecer (el ejercicio).            */

#endif /* CIELOS_H */
