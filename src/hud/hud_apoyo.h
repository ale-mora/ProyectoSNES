/*---------------------------------------------------------------------------------
    hud_apoyo.h — La barra del apoyo en la capa de texto (desde el monasterio).

    PROPÓSITO
      Una fila con el apoyo elegido (R lo cambia) y su barra (juego/apoyo.h), debajo de la
      de vida. Va en la fila 5 del HUD de depuración: con el HUD de depuración encendido,
      esa fila es la de la V y la barra no se dibuja.

    SE CONECTA CON
      Incluye:  sistema/tipos.h, juego/apoyo.h.
      Implementa: hud/hud_apoyo.c.
      Lo usa:   pantallas/pantalla_fase1.c.

    HARDWARE QUE TOCA
      VRAM (el mapa de texto), por la lista de DMA.

    VARIABLES: ninguna (ver hud_apoyo.c).
    → glosario: HUD
---------------------------------------------------------------------------------*/
#ifndef HUD_APOYO_H
#define HUD_APOYO_H

#include "sistema/tipos.h"
#include "juego/apoyo.h"

/* Al entrar en el escenario: lo próximo se dibuja entero. */
void hud_apoyo_iniciar(void);

/* Un frame: si el apoyo está disponible y algo cambió (o cada 16 frames), la fila. */
void hud_apoyo_actualizar(const Apoyo *a);

#endif /* HUD_APOYO_H */
