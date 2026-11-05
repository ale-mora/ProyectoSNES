/*---------------------------------------------------------------------------------
    estado_fase.c — La máquina de estados de la partida en la fase (ver estado_fase.h).

    PROPÓSITO
      Un switch por estado. La cuenta atrás no divide: cuenta frames hasta FASE_SEGUNDO
      y entonces baja un número.

    SE CONECTA CON
      Incluye:  juego/estado_fase.h.
      Lo usan:  ver estado_fase.h.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna (todo va en el EstadoFase del que llama).
    → glosario: Máquina de estados
---------------------------------------------------------------------------------*/
#include "juego/estado_fase.h"

void estado_fase_iniciar(EstadoFase *e)
{
    e->estado = FASE_PREPARADOS;             /* «READY?» antes de jugar (ejercicio 28). */
    e->reloj = 0;
    e->cuenta = FASE_CONTINUE;
    e->libre = 0;
}

/*--- pasar: cambia de estado y empieza a contar desde 0 ------------------------------*/
static void pasar(EstadoFase *e, u8 estado)
{
    e->estado = estado;
    e->reloj = 0;
}

u8 estado_fase_actualizar(EstadoFase *e, u8 start, u8 select, u8 ko, u8 fase_vencida)
{
    switch (e->estado)
    {
        case FASE_JUGANDO:
            if (fase_vencida)
            {
                pasar(e, FASE_STAGE_CLEAR);          /* La última oleada ha caído.          */
            }
            else if (ko)
            {
                pasar(e, FASE_KO);                   /* Se queda tumbada un momento.        */
            }
            else if (start)
            {
                pasar(e, FASE_PAUSA);
            }
            return ACCION_NADA;

        case FASE_PAUSA:
            if (select)
            {
                return ACCION_TITULO;                /* Select en pausa: al título.         */
            }
            if (start)
            {
                pasar(e, FASE_JUGANDO);              /* Start: sigue la partida.            */
            }
            return ACCION_NADA;

        case FASE_KO:
            e->reloj++;
            if (e->reloj >= FASE_KO_ESPERA)
            {
                pasar(e, FASE_GAME_OVER);
                e->cuenta = FASE_CONTINUE;
            }
            return ACCION_NADA;

        case FASE_GAME_OVER:
            if (start)
            {
                return ACCION_REINICIAR;             /* CONTINUE: otra vez la fase.         */
            }
            e->reloj++;
            if (e->reloj >= FASE_SEGUNDO)            /* Un segundo más: un número menos.    */
            {
                e->reloj = 0;
                if (e->cuenta == 0)
                {
                    return ACCION_TITULO;            /* Se acabó el tiempo.                 */
                }
                e->cuenta--;
            }
            return ACCION_NADA;

        case FASE_PREPARADOS:                        /* «READY?»: nada lo corta.            */
            e->reloj++;
            if (e->reloj >= FASE_PREPARADOS_FRAMES)
            {
                pasar(e, FASE_JUGANDO);
            }
            return ACCION_NADA;

        default:                                     /* FASE_STAGE_CLEAR                    */
            e->reloj++;
            if (e->reloj >= FASE_STAGE_CLEAR_FRAMES)
            {
                return ACCION_PROXIMAMENTE;
            }
            return ACCION_NADA;
    }
}

u8 estado_fase_mundo_avanza(const EstadoFase *e)
{
    return (u8)(e->estado == FASE_JUGANDO || e->estado == FASE_KO);
}

u8 estado_fase_atenuada(const EstadoFase *e)
{
    return (u8)(e->estado == FASE_PAUSA || e->estado == FASE_GAME_OVER);
}
