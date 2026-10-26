/*---------------------------------------------------------------------------------
    hud_juego.h — El HUD del juego (no el de depuración): la barra de vida de Tarou.

    PROPÓSITO
      Dibuja, en una fila de la capa de texto (BG3), «TAROU [======------]»: una celda por
      cada 2 puntos de vida (VIDA_TAROU = 24 → 12 celdas; «=» llena, «-» vacía). Es provisional
      (docs/diseno-beat-em-up.md §3 prevé un marco superior con barras de los dos protagonistas, la
      activa resaltada, y la barra de ataque; llegará con el arte del HUD y de Bárbara). Usa el
      texto de la consola porque es lo que hay: usa la fila propia 4 del HUD de depuración
      (juego/hud_debug.c: mapa de texto, fila 5; el HUD de la fase usa las filas 0..3). Solo la
      envía a la VRAM cuando la vida cambia, se pide reparar o pasan unos frames (el HUD de
      depuración, al apagarse, vacía el texto y la capa).

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Usa:      juego/hud_debug.h (las filas propias).
      Lo usan:  pantallas/pantalla_fase1.c.

    HARDWARE QUE TOCA
      PPU: bgSetEnable(2) (la capa de texto, siempre encendida mientras dura la fase). VRAM:
      una fila del mapa de texto (VRAM_MAPA_TEXTO) por la lista de DMA, 64 bytes.

    VARIABLES: ver hud_juego.c.
    → glosario: Consola de texto, Tilemap, DMA / HDMA
---------------------------------------------------------------------------------*/
#ifndef HUD_JUEGO_H
#define HUD_JUEGO_H

#include "sistema/tipos.h"

/* Con la pantalla apagada y DESPUÉS de hud_debug_iniciar(): enciende la capa de texto y deja
   la barra por dibujar. */
void hud_juego_iniciar(void);

/* Tras alternar el HUD de depuración (que apaga la capa y vacía el texto): vuelve a encender la
   capa y a dibujar la barra. */
void hud_juego_reparar(void);

/* Una vez por frame, justo DESPUÉS de dma_lista_vaciar() y antes de las subidas de sprites (para
   que su copia vaya la primera): si la vida cambió, hace falta reparar o toca repetirla, escribe
   la barra y la apunta en la lista de DMA. */
void hud_juego_actualizar(u8 vida);

#endif /* HUD_JUEGO_H */
