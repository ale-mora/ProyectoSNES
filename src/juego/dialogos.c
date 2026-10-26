/*---------------------------------------------------------------------------------
    dialogos.c — Los textos de las escenas de la fase 1 (ver dialogos.h).

    PROPÓSITO
      Una tabla por escena (guion-historia.md §11.3 y §11.4). Los cuadros que el guion
      parte con «/» son una línea 1 y una línea 2 del mismo cuadro.

    SE CONECTA CON
      Incluye:  juego/dialogos.h, juego/fase1.h (ESC_*).
      Lo usan:  tests/prueba_dialogos.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (tablas en ROM, static const)
      lucinda, fenrir, rex, jefe, villano   Cuadro[]   los cuadros de cada escena
    → glosario: Diálogo
---------------------------------------------------------------------------------*/
#include "juego/dialogos.h"
#include "juego/fase1.h"

typedef struct
{
    const char *orador;
    const char *linea1;
    const char *linea2;
} Cuadro;

static const Cuadro lucinda[3] =
{
    {"LUCINDA", "You two. Fight me. Now.", ""},
    {"BARBARA", "We don't have time!", ""},
    {"LUCINDA", "Monastery? Rex's food?", "I'm coming."},
};

static const Cuadro fenrir[1] =
{
    {"FENRIR", "WHEEE!", ""},
};

static const Cuadro rex[4] =
{
    {"REX", "That is no toy, young one.", ""},
    {"REX", "It is an ancient god.", "Nikoraemon wants it."},
    {"REX", "Not revenge. Words.", "Let us talk to him."},
    {"BARBARA", "...Talk. Sure.", ""},
};

static const Cuadro jefe[1] =
{
    {"GOMASURI-DONO", "Yes, master! Of course,", "master! Genius, master!"},
};

static const Cuadro villano[1] =
{
    {"NIKORAEMON", "Fools! The relic is mine!", ""},
};

/* La tabla de la escena y cuántos cuadros tiene. */
static const Cuadro *tabla(u8 escena)
{
    switch (escena)
    {
    case ESC_LUCINDA:
        return lucinda;
    case ESC_FENRIR_GAG:
        return fenrir;
    case ESC_REX:
        return rex;
    case ESC_JEFE_ENTRA:
        return jefe;
    case ESC_VILLANO_HUYE:
        return villano;
    default:
        return 0;
    }
}

u8 dialogo_cuadros(u8 escena)
{
    switch (escena)
    {
    case ESC_LUCINDA:
        return 3;
    case ESC_FENRIR_GAG:
        return 1;
    case ESC_REX:
        return 4;
    case ESC_JEFE_ENTRA:
        return 1;
    case ESC_VILLANO_HUYE:
        return 1;
    default:
        return 0;
    }
}

const char *dialogo_orador(u8 escena, u8 n)
{
    if (n >= dialogo_cuadros(escena))
        return "";
    return tabla(escena)[n].orador;
}

const char *dialogo_linea1(u8 escena, u8 n)
{
    if (n >= dialogo_cuadros(escena))
        return "";
    return tabla(escena)[n].linea1;
}

const char *dialogo_linea2(u8 escena, u8 n)
{
    if (n >= dialogo_cuadros(escena))
        return "";
    return tabla(escena)[n].linea2;
}
