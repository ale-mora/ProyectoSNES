/*---------------------------------------------------------------------------------
    cielos.c — Tablas HDMA de color (ESQUELETO del ejercicio 12, parte A:
    completa los TODO 1..3 de cielo_atardecer; la solución está en
    ../../solucion/cielos.c).

    PROPÓSITO
      Una tabla HDMA es una lista de entradas «durante N líneas, escribe estos
      bytes». El canal la recorre él solo mientras la PPU dibuja, sin gastar
      CPU. Aquí cada entrada cambia el color 0 de la CGRAM (el fondo, que se ve
      en el cielo transparente de las montañas), así que cada franja de líneas
      sale de un color: un degradado.
      cielo_dia ya está hecha (28 franjas de 8 líneas = 224 líneas, de azul
      noche arriba a celeste en el horizonte). cielo_atardecer es el ejercicio:
      tal como está es una sola franja negra.

    SE CONECTA CON
      Incluye:  src/cielos.h (FRANJA).
      Lo usa:   src/efectos12.c (setModeHdmaColor con una de las dos tablas).
      Modelo:   res/titulo/cielo_grad_data.as del juego (la tabla del cielo del
                título, que gfx4snes -n genera desde cielo.bmp con este mismo formato).

    HARDWARE QUE TOCA
      ninguno: son datos (los lee el canal 6 de HDMA).

    VARIABLES (globales, const)
      cielo_dia        u8[28*5+1]  141 bytes: 28 entradas y el 0 final
      cielo_atardecer  u8[...]     el ejercicio
      OJO: tcc816 pone las tablas const GLOBALES en la WRAM (se copian al
      arrancar; mira el .sym: banco $7F). Al HDMA le da igual leer de la WRAM o
      de la ROM. Ver la unidad 13.
    → glosario: DMA / HDMA, CGRAM
---------------------------------------------------------------------------------*/
#include "cielos.h"

/*--- Cielo de día: 28 franjas de 8 líneas (224, toda la pantalla) ----------------- */
const u8 cielo_dia[] = {
    FRANJA(8,  2,  6, 18),           /* Líneas   0..7:   azul noche.                      */
    FRANJA(8,  3,  7, 18),           /* Líneas   8..15.                                   */
    FRANJA(8,  3,  7, 19),           /* Líneas  16..23.                                   */
    FRANJA(8,  4,  8, 19),           /* Líneas  24..31.                                   */
    FRANJA(8,  5,  9, 20),           /* Líneas  32..39.                                   */
    FRANJA(8,  5, 10, 20),           /* Líneas  40..47.                                   */
    FRANJA(8,  6, 10, 21),           /* Líneas  48..55.                                   */
    FRANJA(8,  7, 11, 21),           /* Líneas  56..63.                                   */
    FRANJA(8,  7, 12, 22),           /* Líneas  64..71.                                   */
    FRANJA(8,  8, 13, 22),           /* Líneas  72..79.                                   */
    FRANJA(8,  9, 13, 23),           /* Líneas  80..87.                                   */
    FRANJA(8,  9, 14, 23),           /* Líneas  88..95.                                   */
    FRANJA(8, 10, 15, 24),           /* Líneas  96..103.                                  */
    FRANJA(8, 11, 16, 24),           /* Líneas 104..111.                                  */
    FRANJA(8, 11, 16, 25),           /* Líneas 112..119.                                  */
    FRANJA(8, 12, 17, 25),           /* Líneas 120..127.                                  */
    FRANJA(8, 13, 18, 26),           /* Líneas 128..135.                                  */
    FRANJA(8, 13, 19, 26),           /* Líneas 136..143.                                  */
    FRANJA(8, 14, 19, 27),           /* Líneas 144..151.                                  */
    FRANJA(8, 15, 20, 27),           /* Líneas 152..159.                                  */
    FRANJA(8, 15, 21, 28),           /* Líneas 160..167.                                  */
    FRANJA(8, 16, 22, 28),           /* Líneas 168..175.                                  */
    FRANJA(8, 17, 22, 29),           /* Líneas 176..183.                                  */
    FRANJA(8, 17, 23, 29),           /* Líneas 184..191.                                  */
    FRANJA(8, 18, 24, 30),           /* Líneas 192..199.                                  */
    FRANJA(8, 19, 25, 30),           /* Líneas 200..207.                                  */
    FRANJA(8, 19, 25, 31),           /* Líneas 208..215.                                  */
    FRANJA(8, 20, 26, 31),           /* Líneas 216..223: celeste en el horizonte.         */
    0x00                             /* Fin de la tabla: el canal para hasta el frame siguiente. */
};

/*--- Cielo de atardecer: EL EJERCICIO ------------------------------------------------
    Debe quedar: 15 franjas de 8 líneas (líneas 0..119) de morado (6, 2, 12) a
    naranja (28, 10, 6); UNA línea blanca (31, 31, 31) justo después; y 13
    franjas de 8 líneas de naranja (28, 10, 6) a amarillo (31, 24, 10).
    La tabla de la solución trae los valores intermedios; si los calculas tú,
    basta con que cada componente suba o baje sin saltos. */
const u8 cielo_atardecer[] = {
    /* TODO 1: las 15 franjas de 8 líneas, de morado a naranja (líneas 0..119). */

    /* TODO 2: la línea blanca: UNA entrada de 1 línea. ¿En qué línea de la
       pantalla debería verse? Escríbelo aquí y compruébalo en Mesen (ver
       ejercicio_12.md, parte A). */

    /* TODO 3: las 13 franjas de 8 líneas, de naranja a amarillo. Y que no falte
       el 0 final (sin él, el canal seguiría leyendo la memoria de detrás). */

    FRANJA(112, 0, 0, 0),            /* Provisional: 112 líneas negras...                 */
    FRANJA(112, 0, 0, 0),            /* ...y otras 112 (bórralas al completar los TODO).  */
    0x00                             /* Fin de la tabla.                                  */
};
