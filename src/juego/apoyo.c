/*---------------------------------------------------------------------------------
    apoyo.c — Los apoyos de la fase 1 (ver apoyo.h).

    PROPÓSITO
      La máquina de estados FUERA → ENTRA → ATACA → SALE → FUERA, la barra y los frames
      de impacto de cada apoyo (IMPACTOS: hasta tres por ataque).

    SE CONECTA CON
      Incluye:  juego/apoyo.h.
      Lo usan:  pantallas/pantalla_fase1.c, hud/hud_apoyo.c y tests/prueba_apoyo.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (constantes)
      IMPACTO  u8[3][4]  los frames del ataque en que golpea cada apoyo (0 = ya no hay más)
      DANIO    u8[3]     el daño de cada impacto
    → glosario: Máquina de estados
---------------------------------------------------------------------------------*/
#include "juego/apoyo.h"

/* Lucinda da tres tajos; Rex y Fenrir, un golpe. El cuarto byte cierra la lista. */
static const u8 IMPACTO[APOYOS][4] = {{12, 20, 28, 0}, {18, 0, 0, 0}, {18, 0, 0, 0}};
static const u8 DANIO[APOYOS] = {2, 2, 3};

void apoyo_iniciar(Apoyo *a, u8 disponible)
{
    a->x = 0;
    a->y = 0;
    a->estado = APOYO_FUERA;
    a->reloj = 0;
    a->barra = 0;
    a->elegido = APOYO_LUCINDA;
    a->disponible = disponible;
    a->evento = APOYO_EVENTO_NINGUNO;
    a->mira_izq = 0;
    a->golpe = 0;
}

void apoyo_cargar(Apoyo *a)
{
    if (!a->disponible || a->estado != APOYO_FUERA)
    {
        return;
    }
    a->barra = (u8)(a->barra + APOYO_CARGA > APOYO_BARRA_MAX ? APOYO_BARRA_MAX : a->barra + APOYO_CARGA);
}

u8 apoyo_listo(const Apoyo *a)
{
    return (u8)(a->disponible && a->estado == APOYO_FUERA && a->barra >= APOYO_BARRA_MAX);
}

void apoyo_cambiar(Apoyo *a)
{
    if (a->estado != APOYO_FUERA)
    {
        return;
    }
    a->elegido++;
    if (a->elegido >= APOYOS)
    {
        a->elegido = APOYO_LUCINDA;
    }
}

u8 apoyo_invocar(Apoyo *a, u16 x, u16 y, u8 mira_izq)
{
    u16 recorrido = (u16)(APOYO_ENTRA_FRAMES * APOYO_ENTRA_PASO);

    if (!apoyo_listo(a))
    {
        return 0;
    }
    a->barra = 0;
    a->estado = APOYO_ENTRA;
    a->reloj = 0;
    a->golpe = 0;
    a->mira_izq = mira_izq;
    a->y = y;
    /* Llega corriendo desde atrás (de espaldas a donde mira) hasta x. */
    a->x = mira_izq ? (u16)(x + recorrido) : (u16)(x > recorrido ? x - recorrido : 0);
    a->evento = APOYO_EVENTO_NINGUNO;
    return 1;
}

void apoyo_actualizar(Apoyo *a)
{
    a->evento = APOYO_EVENTO_NINGUNO;
    if (a->estado == APOYO_FUERA)
    {
        return;
    }
    a->reloj++;
    if (a->estado == APOYO_ENTRA)
    {
        a->x = a->mira_izq ? (u16)(a->x - APOYO_ENTRA_PASO) : (u16)(a->x + APOYO_ENTRA_PASO);
        if (a->reloj >= APOYO_ENTRA_FRAMES)
        {
            a->estado = APOYO_ATACA;
            a->reloj = 0;
        }
    }
    else if (a->estado == APOYO_ATACA)
    {
        if (a->golpe < 3 && IMPACTO[a->elegido][a->golpe] == a->reloj)
        {
            a->evento = APOYO_EVENTO_IMPACTO;        /* ¡Ahora! La pantalla golpea.          */
            a->golpe++;
        }
        if (a->reloj >= APOYO_ATAQUE_FRAMES)
        {
            a->estado = APOYO_SALE;
            a->reloj = 0;
        }
    }
    else if (a->reloj >= APOYO_SALE_FRAMES)
    {
        a->estado = APOYO_FUERA;                     /* Se fue: vuelve la protagonista.     */
        a->reloj = 0;
    }
}

u8 apoyo_activo(const Apoyo *a)
{
    return (u8)(a->estado != APOYO_FUERA);
}

u8 apoyo_danio(const Apoyo *a)
{
    return DANIO[a->elegido];
}

u8 apoyo_derriba(const Apoyo *a)
{
    /* Rex y Fenrir derriban siempre; Lucinda, con el último tajo. */
    return (u8)(a->elegido != APOYO_LUCINDA || IMPACTO[a->elegido][a->golpe] == 0);
}

u8 apoyo_cuadro(const Apoyo *a)
{
    u8 c = 0, r = a->reloj;

    if (a->estado == APOYO_ENTRA)
    {
        return (u8)((r >> 2) & 3);                   /* 4 cuadros de carrera, 4 frames.     */
    }
    if (a->estado == APOYO_SALE)
    {
        return 5;                                    /* Se queda en el último del ataque.   */
    }
    while (r >= 6 && c < 5)                          /* 6 cuadros de 6 frames (sin dividir). */
    {
        r = (u8)(r - 6);
        c++;
    }
    return c;
}

u8 apoyo_atacando(const Apoyo *a)
{
    return (u8)(a->estado != APOYO_ENTRA);
}
