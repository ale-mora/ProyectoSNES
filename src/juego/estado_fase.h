/*---------------------------------------------------------------------------------
    estado_fase.h — El estado de la partida en la fase: jugando, en pausa, KO, GAME OVER
    con CONTINUE y STAGE CLEAR.

    PROPÓSITO
      Lógica pura (sin snes.h, probada en el PC: tests/prueba_estado_fase.c) de lo que
      pasa alrededor del combate (decisiones del 2026-11-05):
        - Start pausa (el mundo se congela y la pantalla queda a media luz) y Start
          reanuda; Select en pausa vuelve al título.
        - Sin vida, la protagonista cae KO; un momento después, GAME OVER con
          «CONTINUE? 9» que baja cada segundo: Start antes del 0 vuelve a empezar la
          fase con el mismo personaje; al llegar a 0, al título.
        - Vencida la oleada de la última zona: STAGE CLEAR unos 3 s y a la pantalla
          «STAGE 2 / COMING SOON».
      La pantalla (pantallas/pantalla_fase1.c) le pasa cada frame los botones recién
      pulsados y lo que ha ocurrido, y obedece a la acción que le devuelve.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Implementa: juego/estado_fase.c.
      Lo usan:  pantallas/pantalla_fase1.c, tests/prueba_estado_fase.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (campos de EstadoFase; 4 bytes)
      estado    u8  FASE_JUGANDO, FASE_PAUSA, FASE_KO, FASE_GAME_OVER, FASE_STAGE_CLEAR
      reloj     u8  frames en el estado (o en el segundo en curso de la cuenta atrás)
      cuenta    u8  el número de CONTINUE (9..0)
    → glosario: Máquina de estados
---------------------------------------------------------------------------------*/
#ifndef ESTADO_FASE_H
#define ESTADO_FASE_H

#include "sistema/tipos.h"

#define FASE_KO_ESPERA 60                /* Tumbada 1 s antes del GAME OVER.              */
#define FASE_SEGUNDO 60                  /* Frames por número de la cuenta atrás.         */
#define FASE_CONTINUE 9                  /* La cuenta empieza en 9.                       */
#define FASE_STAGE_CLEAR_FRAMES 180      /* STAGE CLEAR en pantalla, 3 s.                 */

typedef enum
{
    FASE_JUGANDO = 0,
    FASE_PAUSA,
    FASE_KO,                             /* Caída sin vida: el mundo sigue un momento.    */
    FASE_GAME_OVER,                      /* «GAME OVER / CONTINUE? n».                    */
    FASE_STAGE_CLEAR
} EstadoFaseId;

typedef enum
{
    ACCION_NADA = 0,
    ACCION_TITULO,                       /* Salir al título.                              */
    ACCION_REINICIAR,                    /* Empezar la fase de nuevo (continue).          */
    ACCION_PROXIMAMENTE                  /* A «STAGE 2 / COMING SOON».                    */
} AccionFase;

typedef struct
{
    u8 estado;
    u8 reloj;
    u8 cuenta;
    u8 libre;
} EstadoFase;

/* Jugando, desde cero. */
void estado_fase_iniciar(EstadoFase *e);

/* Un frame. `start` y `select` son los botones recién pulsados; `ko` vale 1 cuando la
   protagonista está caída sin vida y ya no se mueve; `fase_vencida` vale 1 cuando ya no
   queda ninguna oleada. Devuelve la acción para la pantalla. */
u8 estado_fase_actualizar(EstadoFase *e, u8 start, u8 select, u8 ko, u8 fase_vencida);

/* 1 si el mundo (cuerpo, ninjas, cámara) avanza este frame: jugando o en el KO. */
u8 estado_fase_mundo_avanza(const EstadoFase *e);

/* 1 si la pantalla debe estar a media luz (pausa y GAME OVER). */
u8 estado_fase_atenuada(const EstadoFase *e);

#endif /* ESTADO_FASE_H */
