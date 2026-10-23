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
      la fila del mapa de texto que se preparó en el frame, apuntada en la
      lista de DMA del VBlank (no se usa consoleDrawText: su formateo era tan
      caro que el juego perdía VBlanks).

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

/*--- Filas propias (las usa la ROM de prueba de carga, pantallas/prueba_carga.c) ---
    Para escribir otras filas con el mismo método barato: empezar (la deja en blanco
    y da un puntero a su primera celda), texto/numero (escriben y devuelven dónde
    sigue) y enviar (apunta la fila, 64 bytes, en la lista de DMA). Filas 0..7. */
u16 *hud_debug_empezar(u8 fila);
u16 *hud_debug_texto(u16 *p, const char *s);
u16 *hud_debug_numero(u16 *p, s16 valor);
void hud_debug_enviar(u8 fila);

#endif /* HUD_DEBUG_H */
