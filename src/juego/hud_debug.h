/*---------------------------------------------------------------------------------
    hud_debug.h — HUD de depuración: los números del protagonista en pantalla.

    PROPÓSITO
      Mostrar en la capa de texto (BG3 del hardware, fondo 2 del SDK) la
      posición, la velocidad, el estado, la espada, la dirección, la cámara y
      la caja de golpe, en vivo. No es parte del juego: es la herramienta para
      afinar las constantes de docs/diseno-demo-jugable.md §5 en Mesen y para
      que se "vea" el punto fijo funcionando. SELECT lo enciende y apaga;
      arranca apagado (§9).

    SE CONECTA CON
      Incluye:  juego/tarou.h.
      Lo usan:  pantallas/pantalla_nivel1.c (iniciar, alternar con SELECT,
                dibujar cada frame).

    HARDWARE QUE TOCA
      PPU: encendido y apagado del fondo 2 (bgSetEnable / bgSetDisable). VRAM:
      seis filas del mapa de texto, escritas por DMA en cada VBlank (no se usa
      consoleDrawText: su formateo era tan caro que el juego perdía VBlanks).

    VARIABLES: ninguna (ver hud_debug.c).
    → glosario: HUD de depuración, Consola de texto, Punto fijo
---------------------------------------------------------------------------------*/
#ifndef HUD_DEBUG_H
#define HUD_DEBUG_H

#include "juego/tarou.h"

/* Deja el HUD apagado y el texto limpio (al entrar en el nivel). */
void hud_debug_iniciar(void);

/* Enciende el HUD si estaba apagado y al revés (SELECT). */
void hud_debug_alternar(void);

/* Prepara en RAM las seis filas de §9 si el HUD está encendido. Una vez por frame. */
void hud_debug_dibujar(const Tarou *t, u16 camara);

/* Copia esas filas a la VRAM por DMA. Llamar justo después de WaitForVBlank()
   (desde pantalla_nivel1_vblank), que es cuando se puede escribir en la VRAM. */
void hud_debug_vblank(void);

#endif /* HUD_DEBUG_H */
