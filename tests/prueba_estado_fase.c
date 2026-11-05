/*---------------------------------------------------------------------------------
    prueba_estado_fase.c — Pruebas de juego/estado_fase.c (pausa, KO, GAME OVER con
    CONTINUE y STAGE CLEAR).

    PROPÓSITO
      Fijar las reglas del 2026-11-05: Start pausa y reanuda, Select en pausa sale al
      título; el KO pasa al GAME OVER al cabo de FASE_KO_ESPERA; la cuenta baja de 9 a 0
      un número por segundo y al acabar sale al título; Start en el GAME OVER reinicia;
      vencida la fase, STAGE CLEAR y COMING SOON a los 3 s.

    SE CONECTA CON
      Incluye:  minitest.h, juego/estado_fase.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_estado_fase).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Máquina de estados
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/estado_fase.h"

void pruebas_estado_fase(void)
{
    EstadoFase e;
    u16 f;
    u8 a = ACCION_NADA;

    /* Jugando: el mundo avanza y no está atenuada. Start pausa. */
    estado_fase_iniciar(&e);
    COMPROBAR(estado_fase_mundo_avanza(&e) && !estado_fase_atenuada(&e));
    COMPROBAR(estado_fase_actualizar(&e, 1, 0, 0, 0) == ACCION_NADA && e.estado == FASE_PAUSA);
    COMPROBAR(!estado_fase_mundo_avanza(&e) && estado_fase_atenuada(&e));
    /* En pausa no pasa nada sin botones; Start reanuda. */
    for (f = 0; f < 500; f++)
    {
        COMPROBAR(estado_fase_actualizar(&e, 0, 0, 1, 1) == ACCION_NADA && e.estado == FASE_PAUSA);
    }
    estado_fase_actualizar(&e, 1, 0, 0, 0);
    COMPROBAR(e.estado == FASE_JUGANDO);
    /* Select en pausa: al título; jugando, Select no hace nada aquí (es el HUD). */
    COMPROBAR(estado_fase_actualizar(&e, 0, 1, 0, 0) == ACCION_NADA && e.estado == FASE_JUGANDO);
    estado_fase_actualizar(&e, 1, 0, 0, 0);
    COMPROBAR(estado_fase_actualizar(&e, 0, 1, 0, 0) == ACCION_TITULO);

    /* KO: el mundo sigue un momento y luego GAME OVER, ya atenuada. */
    estado_fase_iniciar(&e);
    estado_fase_actualizar(&e, 0, 0, 1, 0);
    COMPROBAR(e.estado == FASE_KO && estado_fase_mundo_avanza(&e));
    COMPROBAR(estado_fase_actualizar(&e, 1, 0, 1, 0) == ACCION_NADA && e.estado == FASE_KO);   /* Start no pausa. */
    for (f = 1; f < FASE_KO_ESPERA; f++)              /* (El Start de arriba ya contó un frame.) */
    {
        estado_fase_actualizar(&e, 0, 0, 1, 0);
    }
    COMPROBAR(e.estado == FASE_GAME_OVER && e.cuenta == FASE_CONTINUE && estado_fase_atenuada(&e));
    /* La cuenta: un número por segundo; tras el 0, al título (10 s en total). */
    for (f = 0; f < (u16)FASE_SEGUNDO * FASE_CONTINUE; f++)
    {
        COMPROBAR(estado_fase_actualizar(&e, 0, 0, 1, 0) == ACCION_NADA);
    }
    COMPROBAR(e.cuenta == 0);
    for (f = 0; f < FASE_SEGUNDO && a == ACCION_NADA; f++)
    {
        a = estado_fase_actualizar(&e, 0, 0, 1, 0);
    }
    COMPROBAR(a == ACCION_TITULO && f == FASE_SEGUNDO);
    /* Start durante la cuenta: continuar. */
    estado_fase_iniciar(&e);
    e.estado = FASE_GAME_OVER;
    e.cuenta = 3;
    COMPROBAR(estado_fase_actualizar(&e, 1, 0, 1, 0) == ACCION_REINICIAR);

    /* Fase vencida: STAGE CLEAR 3 s y COMING SOON; el mundo se congela. */
    estado_fase_iniciar(&e);
    estado_fase_actualizar(&e, 0, 0, 0, 1);
    COMPROBAR(e.estado == FASE_STAGE_CLEAR && !estado_fase_mundo_avanza(&e) && !estado_fase_atenuada(&e));
    a = ACCION_NADA;
    for (f = 0; f < FASE_STAGE_CLEAR_FRAMES + 5 && a == ACCION_NADA; f++)
    {
        a = estado_fase_actualizar(&e, 1, 1, 0, 1);  /* Ni Start ni Select la cortan.       */
    }
    COMPROBAR(a == ACCION_PROXIMAMENTE && f == FASE_STAGE_CLEAR_FRAMES);
}
