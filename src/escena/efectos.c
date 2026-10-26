/*---------------------------------------------------------------------------------
    efectos.c — Los registros de la PPU detrás de cada pista (ver efectos.h).

    PROPÓSITO
      La parte interesante es el DESTELLO. La SNES tiene "ventanas": rangos de
      columnas [izquierda, derecha] que se pueden cambiar en CADA LÍNEA con
      HDMA. Si en cada línea la ventana empieza un píxel más a la izquierda que
      en la de arriba, sale una banda diagonal. Y la matemática de color, puesta
      a "solo dentro de la ventana", suma un color fijo ahí: la banda brilla.

      Para MOVER la banda no se reescriben las líneas cada frame. Se usa HDMA
      INDIRECTO: la tabla del canal no lleva los datos, lleva un PUNTERO a
      ellos, y en modo "repetir" lee un par (izq, der) nuevo en cada línea
      avanzando por la memoria. Los datos son una lista larga de pares que se
      calcula UNA vez (efectos_destello) y en la que el par j vale
      izq = 255 + x_min - ancho - j: la línea y lee el par (inicio + y), así que
      cada línea queda 1 px a la izquierda de la anterior, y elegir dónde
      empieza a leer el canal (`inicio`) desplaza la banda entera. Cada frame
      solo se cambian los 2 bytes de ese puntero: 0 líneas de trabajo.

      Fuera del rectángulo (líneas de arriba y de abajo, columnas fuera de
      [x_min, x_max]) la ventana queda VACÍA (izquierda 255 > derecha 0):
      el negro del logo Nintendo es un color opaco, y sin esto la luz lo
      aclararía también.

    SE CONECTA CON
      Incluye:  escena/efectos.h.
      Lo usan:  pantallas/pantalla_nintendo.c.

    HARDWARE QUE TOCA
      Ver efectos.h. Registros de un byte: ventana $2125-$2127, color
      $2130-$2132, mosaico $2106, HDMA $4360-$4367 y $420C.

    VARIABLES (estáticas, en la WRAM)
      fondos    u8                  bits de los fondos con mosaico y luz
      destello  u8                  1 si hay banda preparada
      pares     u8[2 + 2*(256+64)]  los pares (izq, der); los 2 primeros, el
                                    par "vacío" (255, 0)
      tabla     u8[10]              la tabla HDMA indirecta: 3 entradas de
                                    (líneas, puntero) y el 0 final

    → glosario: Ventana, Matemática de color, DMA / HDMA, Mosaico, Motor de escenas
---------------------------------------------------------------------------------*/
#include "escena/efectos.h"

#define BANCO_WRAM 0x7E                              /* Banco de las variables de C.        */
#define REG_DASB6 (*(vuint8 *)0x4367)                /* Banco de los datos (HDMA indirecto); */
                                                     /* el SDK no lo define.                */
#define PARES_DESTELLO (256 + EFECTOS_ALTO_MAX)      /* 256 posiciones + las líneas.        */

static u8 fondos;
static u8 destello;
static u8 pares[2 + 2 * PARES_DESTELLO];
static u8 tabla[10];

void efectos_preparar(u8 bits_fondos)
{
    fondos = bits_fondos;
    destello = 0;
    REG_MOSAIC = 0;                                  /* Sin mosaico hasta el primer VBlank. */
}

void efectos_destello(u8 arriba, u8 alto, u8 x_min, u8 x_max, u8 ancho)
{
    u8 *d = pares;
    s16 izq = (s16)(255 + x_min - ancho);            /* El par 0: banda justo a la derecha  */
    s16 der;                                         /* del todo (ver el PROPÓSITO).        */
    u16 n;
    u16 dir;

    *d++ = 255;                                      /* Par "vacío": izquierda > derecha.   */
    *d++ = 0;
    for (n = (u16)(256 + alto); n != 0; n--, izq--)  /* Un par por posición posible.        */
    {
        der = (s16)(izq + ancho - 1);
        if (der < x_min || izq > x_max)              /* Fuera del rectángulo: vacía.        */
        {
            *d++ = 255;
            *d++ = 0;
        }
        else                                         /* Dentro: recortada a [x_min, x_max]. */
        {
            *d++ = (u8)(izq < x_min ? x_min : izq);
            *d++ = (u8)(der > x_max ? x_max : der);
        }
    }

    /* La tabla del canal: (líneas, puntero) x 3 y un 0. Bit 7 de "líneas" =
       repetir: un par nuevo en cada línea; sin él, un par y se mantiene.
       La primera entrada cubre `arriba` líneas: con el fondo en scroll vertical
       -1 (SCROLL_Y_FILA0, desde el 2026-10-23) la fila n del logo cae en la
       línea n y la ventana coincide con él. Hasta entonces el fondo iba en
       scroll 0, el logo se veía 1 px más arriba y hacía falta `arriba - 1`
       (la raya gris bajo el logo del 2026-10-21 venía de ese desfase del
       fondo, no del HDMA; unidad 12 de aprendizaje/). */
    dir = (u16)pares;                                /* Dirección del par vacío.            */
    tabla[0] = (u8)arriba;                           /* Las líneas de arriba: vacía.        */
    tabla[1] = (u8)dir;
    tabla[2] = (u8)(dir >> 8);
    tabla[3] = (u8)(0x80 | alto);                    /* Las del rectángulo: un par por      */
    tabla[4] = (u8)(dir + 2);                        /* línea (el puntero lo mueve          */
    tabla[5] = (u8)((u16)(dir + 2) >> 8);            /* efectos_aplicar).                   */
    tabla[6] = 1;                                    /* Debajo: vacía hasta el final.       */
    tabla[7] = (u8)dir;
    tabla[8] = (u8)(dir >> 8);
    tabla[9] = 0;                                    /* Fin de la tabla.                    */

    /* La ventana 1 decide dónde hay matemática de color, y la matemática suma
       el color fijo (COLDATA) a los fondos elegidos, solo dentro de ella. */
    REG_WOBJSEL = 0x20;                              /* Ventana de color = ventana 1.       */
    REG_CGWSEL = 0x10;                               /* Matemática solo DENTRO de la ventana. */
    REG_CGADSUB = fondos;                            /* Sumar (sin restar ni dividir).      */
    REG_COLDATA = 0xE0;                              /* Color fijo negro: aún no suma nada. */

    /* Canal 6 en HDMA indirecto: modo 1 = dos bytes por línea, a $2126 (WH0,
       borde izquierdo) y $2127 (WH1, borde derecho). */
    REG_DMAP6 = 0x41;                                /* Bit 6: indirecto. Modo 1.           */
    REG_BBAD6 = 0x26;                                /* Destino: WH0 (y WH1).               */
    REG_A1T6LH = (u16)tabla;                         /* La tabla...                         */
    REG_A1B6 = BANCO_WRAM;                           /* ...en la WRAM.                      */
    REG_DASB6 = BANCO_WRAM;                          /* Los pares también.                  */
    REG_HDMAEN = HDMA_CHANNEL6;                      /* Empieza en el próximo frame.        */
    destello = 1;
}

void efectos_aplicar(const Escena *e)
{
    u16 dir;

    setBrightness(ESCENA_VALOR(e, PISTA_BRILLO));    /* 0 = pantalla apagada (forced blank). */
    REG_MOSAIC = (u8)((ESCENA_VALOR(e, PISTA_MOSAICO) << 4) | fondos);   /* Tamaño | fondos. */
    if (destello)
    {
        REG_COLDATA = (u8)(0xE0 | (ESCENA_VALOR(e, PISTA_LUZ) & 0x1F));  /* R, G y B a la vez. */
        /* Avance 255 = leer desde el par 0 (banda a la derecha del todo);
           avance 0 = desde el par 255 (a la izquierda). +2: saltar el vacío. */
        dir = (u16)((u16)pares + 2 + ((u16)(255 - ESCENA_VALOR(e, PISTA_DESTELLO)) << 1));
        tabla[4] = (u8)dir;                          /* Solo cambia este puntero.           */
        tabla[5] = (u8)(dir >> 8);
    }
}

void efectos_terminar(void)
{
    REG_HDMAEN = 0;                                  /* Sin HDMA (el título pone el suyo).  */
    REG_CGWSEL = 0;                                  /* Sin matemática de color.            */
    REG_CGADSUB = 0;
    REG_COLDATA = 0xE0;                              /* Color fijo a negro.                 */
    REG_WOBJSEL = 0;                                 /* Sin ventana de color.               */
    REG_WH0 = 0;
    REG_WH1 = 0;
    REG_MOSAIC = 0;                                  /* Sin mosaico.                        */
    destello = 0;
}
