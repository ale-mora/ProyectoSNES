/*---------------------------------------------------------------------------------
    niveles.c — Carga de niveles por tiles y colisión con el escenario (Etapa 2/3).

    PROPÓSITO
      PENDIENTE: Etapa 2/3 — sustituir el suelo plano de juego/nivel.c por mapas de tiles hechos en Tiled (tileset + .tmx), con scroll por columnas y colisión por tiles, siguiendo el motor de mapas de LikeMario (examples/likemario).
      Este archivo es un STUB: compila, reserva el sitio del subsistema y documenta
      el contrato previsto. No contiene lógica; la única función real,
      niveles_pendiente(), existe para que el enlazador vea el objeto.

    SE CONECTA CON
      Incluirá: <snes.h> (mapLoad, mapUpdate, mapVblank), juego/nivel.h, juego/camara.h.
      Lo usarán: pantallas/pantalla_nivel1.c, juego/tarou.c (nivel_suelo_en pasará a consultar el mapa).

    HARDWARE QUE TOCA
      VRAM (tiles y mapas de las capas) por DMA en cada VBlank, como hace LikeMario.

    VARIABLES (globales y estáticas de este archivo)
      ninguna todavía.

    → glosario: Stub, Nivel, Escenario, Tilemap, Suelo
---------------------------------------------------------------------------------*/
#include "niveles/niveles.h"

void niveles_pendiente(void)
{
    /* PENDIENTE: Etapa 2/3. Ver el contrato previsto en niveles.h. */
}
