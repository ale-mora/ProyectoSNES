/*---------------------------------------------------------------------------------
    oam_piezas.h — Escribir de un tirón las piezas de 16x16 de un metasprite en la OAM.

    PROPÓSITO
      La prueba de carga (docs/diseno-beat-em-up.md §9) midió que llenar la OAM
      con oamSet + oamSetEx del SDK cuesta unas 5 líneas del haz POR PIEZA desde
      C (tcc816): con 3 ninjas de ~12 piezas el frame no cabía. Esta rutina, en
      ensamblador (oam_piezas.asm), escribe las n piezas seguidas directamente
      en el búfer de la OAM del SDK (oamMemory, que la NMI copia a la OAM en cada
      VBlank): x, y, tile y atributos de cada una, y limpia sus 2 bits de la tabla
      alta (x < 256, tamaño pequeño).
      Las piezas vienen en una tabla de pares (dx, dy) y sus tiles están en la
      VRAM empaquetados como los deja scripts/preparar_prueba_carga.py: 8 piezas
      por par de filas (tile + 2 cada una; tras 8, el par siguiente).

    SE CONECTA CON
      Implementa: juego/oam_piezas.asm.
      Lo usan:  pantallas/prueba_carga.c.

    HARDWARE QUE TOCA
      ninguno directamente: escribe en oamMemory (WRAM), el búfer del SDK.

    VARIABLES: ninguna.
    → glosario: OAM, Metasprite, Ensamblador 65816
---------------------------------------------------------------------------------*/
#ifndef OAM_PIEZAS_H
#define OAM_PIEZAS_H

#include "sistema/tipos.h"

/* Escribe n piezas de 16x16 desde la entrada de la OAM entrada/4 (`entrada` ya
   multiplicada por 4, como la usa el SDK). Pieza k: x = x0 - dx (espejo != 0) o
   x0 + dx; y = y0 + dy; tile = tile_bajo + 2k (+16 tras cada 8); atributos =
   el byte vhoopppN (volteo, prioridad, paleta y el bit 8 del tile), igual para
   todas. Las x tienen que quedar en 0..255. */
void oam_piezas(u16 entrada, u16 x0, u16 y0, u16 tile_bajo, u16 atributos,
                const u8 *piezas, u16 n, u16 espejo);

#endif /* OAM_PIEZAS_H */
