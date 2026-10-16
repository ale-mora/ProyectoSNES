/*---------------------------------------------------------------------------------
    hud.h — HUD del juego: vida y objetos (Etapa 3).

    PROPÓSITO
      PENDIENTE: Etapa 3 — marcador visible para el jugador (vida, objetos, quizá tiempo), en la capa BG3; distinto de juego/hud_debug.c, que es una herramienta de desarrollo.
      Este archivo es un STUB: compila, reserva el sitio del subsistema y documenta
      el contrato previsto. No contiene lógica; la única función real,
      hud_pendiente(), existe para que el enlazador vea el objeto.

    SE CONECTA CON
      Incluirá: sistema/comun.h (VRAM_MAPA_TEXTO), sistema/tipos.h.
      Lo usarán: pantallas/pantalla_nivel1.c.

    HARDWARE QUE TOCA
      VRAM (mapa de texto de BG3) por DMA, como hud_debug.c.

    VARIABLES (globales y estáticas de este archivo)
      ninguna todavía.

    → glosario: Stub, HUD de depuración, Tilemap
---------------------------------------------------------------------------------*/
#ifndef HUD_H
#define HUD_H

#include "sistema/tipos.h"

/* Contrato previsto (Etapa 3):
void hud_cargar(void);
void hud_dibujar(u8 vida, u8 objetos); */

/* Única función real del stub: no hace nada. Se quitará al implementar el módulo. */
void hud_pendiente(void);

#endif /* HUD_H */
