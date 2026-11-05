/*---------------------------------------------------------------------------------
    apoyo.h — Los apoyos de la fase 1: la barra de ataque y la invocación con X.

    PROPÓSITO
      Desde el monasterio (Rex se une tras su escena) la protagonista tiene una BARRA DE
      APOYO que se llena al conectar golpes (APOYO_CARGA por golpe, APOYO_BARRA_MAX llena).
      Con la barra llena, X invoca al apoyo elegido (R cambia: Lucinda → Rex → Fenrir) y la
      barra se vacía. La invocación es un «súper» de tres tiempos en los que el mundo se
      congela (como una parada de impacto larga) y solo se mueve el apoyo:
        ENTRA   APOYO_ENTRA_FRAMES: llega corriendo hasta el sitio de la protagonista
                (que se retira: la sustituye en pantalla, como un relevo)
        ATACA   APOYO_ATAQUE_FRAMES: su ataque; en los frames de IMPACTO avisa
                (evento APOYO_EVENTO_IMPACTO) y la pantalla golpea a todos los enemigos:
                  Lucinda  tres tajos (destellos blancos), 2 + 2 + 2 y el último derriba
                  Rex      una onda (mosaico), 2 y derriba a todos
                  Fenrir   una embestida (la pantalla tiembla), 3 y derriba a todos
        SALE    APOYO_SALE_FRAMES: parpadea y vuelve la protagonista
      Lógica pura, sin snes.h (tests/prueba_apoyo.c).

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Implementa: juego/apoyo.c.
      Lo usan:  pantallas/pantalla_fase1.c, hud/hud_apoyo.c y tests/prueba_apoyo.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (campos de Apoyo; 16 bytes)
      x, y        u16  dónde está el apoyo (en el escenario)
      estado      u8   APOYO_FUERA / ENTRA / ATACA / SALE
      reloj       u8   frames en el estado
      barra       u8   0..APOYO_BARRA_MAX
      elegido     u8   APOYO_LUCINDA / REX / FENRIR
      disponible  u8   1 desde el monasterio
      evento      u8   APOYO_EVENTO_* de este frame
      mira_izq    u8   hacia dónde mira
      golpe       u8   cuántos impactos lleva este ataque (0..2)
    → glosario: Máquina de estados
---------------------------------------------------------------------------------*/
#ifndef APOYO_H
#define APOYO_H

#include "sistema/tipos.h"

#define APOYO_LUCINDA 0
#define APOYO_REX 1
#define APOYO_FENRIR 2
#define APOYOS 3

#define APOYO_FUERA 0
#define APOYO_ENTRA 1
#define APOYO_ATACA 2
#define APOYO_SALE 3

#define APOYO_EVENTO_NINGUNO 0
#define APOYO_EVENTO_IMPACTO 1

#define APOYO_BARRA_MAX 32           /* 16 golpes conectados.                              */
#define APOYO_CARGA 2
#define APOYO_ENTRA_FRAMES 24        /* 3 px por frame: 72 px de carrera.                  */
#define APOYO_ENTRA_PASO 3
#define APOYO_ATAQUE_FRAMES 36       /* 6 cuadros de 6 frames.                             */
#define APOYO_SALE_FRAMES 16
#define APOYO_GOLPE_ID 250           /* Ids de golpe de los impactos (250, 251, 252).      */

typedef struct
{
    u16 x, y;
    u8 estado;
    u8 reloj;
    u8 barra;
    u8 elegido;
    u8 disponible;
    u8 evento;
    u8 mira_izq;
    u8 golpe;
    u8 libre[4];                     /* Relleno hasta 16 bytes.                            */
} Apoyo;

/* Sin apoyo en pantalla, barra vacía; `disponible` = 1 si ya se puede invocar. */
void apoyo_iniciar(Apoyo *a, u8 disponible);

/* Un golpe de la protagonista conectó: sube la barra (si hay apoyo y no está en pantalla). */
void apoyo_cargar(Apoyo *a);

/* 1 si la barra está llena (se puede invocar). */
u8 apoyo_listo(const Apoyo *a);

/* R: el apoyo siguiente (solo fuera de pantalla). */
void apoyo_cambiar(Apoyo *a);

/* X: con la barra llena, el apoyo entra hacia (x, y) mirando como la protagonista.
   Devuelve 1 si empezó. */
u8 apoyo_invocar(Apoyo *a, u16 x, u16 y, u8 mira_izq);

/* Un frame (con el apoyo en pantalla): avanza, mueve y deja el evento del frame. */
void apoyo_actualizar(Apoyo *a);

/* 1 mientras el apoyo está en pantalla (el mundo, congelado). */
u8 apoyo_activo(const Apoyo *a);

/* El daño del impacto que acaba de avisar y si derriba. */
u8 apoyo_danio(const Apoyo *a);
u8 apoyo_derriba(const Apoyo *a);

/* El cuadro (0..5) de su animación y si es la de ataque (1) o la de entrada (0). */
u8 apoyo_cuadro(const Apoyo *a);
u8 apoyo_atacando(const Apoyo *a);

#endif /* APOYO_H */
