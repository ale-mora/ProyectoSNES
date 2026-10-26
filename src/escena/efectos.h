/*---------------------------------------------------------------------------------
    efectos.h — Lleva los valores de una escena a los registros de la PPU.

    PROPÓSITO
      escena.c calcula NÚMEROS (brillo 9, mosaico 3, destello en 140...); este
      módulo los convierte en lo que se ve, escribiendo los registros en el
      VBlank. Hoy sabe hacer cuatro cosas, una por pista:
        PISTA_BRILLO    el brillo de la pantalla (INIDISP, con setBrightness).
        PISTA_MOSAICO   el mosaico de los fondos elegidos (MOSAIC).
        PISTA_DESTELLO  dónde está una banda de luz diagonal: una VENTANA cuyos
                        bordes cambian línea a línea por HDMA.
        PISTA_LUZ       cuánta luz suma la banda: MATEMÁTICA DE COLOR, que suma
                        un color fijo (COLDATA) a los píxeles de dentro de la
                        ventana.
      Uso: efectos_preparar() y, si hay destello, efectos_destello() en el
      _iniciar de la pantalla (con el brillo a 0); efectos_aplicar() en su
      _vblank; efectos_terminar() ANTES de cambiar de pantalla, para que la
      siguiente no herede la ventana, la luz ni el mosaico.

    SE CONECTA CON
      Incluye:  snes.h, escena/escena.h.
      Lo usan:  pantallas/pantalla_nintendo.c.

    HARDWARE QUE TOCA
      INIDISP (brillo), MOSAIC, WOBJSEL / WH0 / WH1 (ventana 1), CGWSEL /
      CGADSUB / COLDATA (matemática de color), canal 6 de DMA en modo HDMA
      indirecto ($4360-$4367, HDMAEN). El canal 6 es el mismo del cielo del
      título: nunca están a la vez.

    VARIABLES: las de efectos.c.
    → glosario: Motor de escenas, Ventana, Matemática de color, DMA / HDMA, Mosaico
---------------------------------------------------------------------------------*/
#ifndef EFECTOS_H
#define EFECTOS_H

#include <snes.h>
#include "escena/escena.h"

/* Qué fondos reciben el mosaico y la luz: un bit por fondo, como en los
   registros (bit 0 = fondo 0, bit 1 = fondo 1...). */
void efectos_preparar(u8 fondos);

/* Prepara la banda de luz diagonal ("/") sobre el rectángulo de las líneas
   [arriba, arriba + alto) y las columnas [x_min, x_max], con `ancho` px.
   Fuera de ese rectángulo la banda no existe. 2 <= arriba, alto <= EFECTOS_ALTO_MAX. */
#define EFECTOS_ALTO_MAX 64
void efectos_destello(u8 arriba, u8 alto, u8 x_min, u8 x_max, u8 ancho);

/* En el VBlank: escribe los valores actuales de la escena en la PPU. */
void efectos_aplicar(const Escena *e);

/* Apaga todo lo que se encendió (deja el brillo como esté). */
void efectos_terminar(void);

#endif /* EFECTOS_H */
