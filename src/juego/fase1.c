/*---------------------------------------------------------------------------------
    fase1.c — Los datos de la fase 1 (ver fase1.h).

    PROPÓSITO
      Las tablas de las cuatro zonas, con los números que decidió el autor el
      2026-10-24. Las oleadas suben poco a poco de rango (blanco, amarillo, verde,
      rojo); el pesado, siempre rojo, entra al final del bosque; el jefe, solo.
      Los rompibles reparten la comida por zona (onigiri +16, pescado +64) y una
      vida extra escondida en el último farol de la aldea (decisión 25).
      Son `static const`: así viven en la ROM y no en la WRAM (CLAUDE.md).

    SE CONECTA CON
      Incluye:  juego/fase1.h, juego/cifras.h (RANGO_*).
      Lo usan:  juego/oleadas.c, tests/prueba_oleadas.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (tablas en ROM)
      oleadas_<zona>[]    Oleada     las oleadas de cada zona
      rompibles_<zona>[]  Rompible   los rompibles de cada zona
      zona_<zona>         Zona       su longitud y punteros a las dos tablas
    → glosario: Fase, Zona, Oleada
---------------------------------------------------------------------------------*/
#include "juego/fase1.h"
#include "juego/cifras.h"

#define B TIPO_BASICO
#define R TIPO_RAPIDO
#define P TIPO_PESADO
#define J TIPO_JEFE

/*--- 1. Aldea: 1536 px. Blancos; el rápido llega en la última oleada ----------------*/
static const Oleada oleadas_aldea[3] =
{
    {256, 2, LADO_DERECHA, {{B, RANGO_BLANCO}, {B, RANGO_BLANCO}, {0, 0}}, ESC_NADA, ESC_NADA, {0, 0, 0, 0}},
    {640, 3, LADO_DERECHA, {{B, RANGO_BLANCO}, {B, RANGO_BLANCO}, {B, RANGO_BLANCO}}, ESC_NADA, ESC_NADA, {0, 0, 0, 0}},
    {1024, 3, LADO_ALTERNO, {{B, RANGO_BLANCO}, {B, RANGO_BLANCO}, {R, RANGO_BLANCO}}, ESC_NADA, ESC_NADA, {0, 0, 0, 0}},
};
static const Rompible rompibles_aldea[6] =
{
    {192, ROMP_BARRIL, CONT_ONIGIRI},
    {416, ROMP_CAJA, CONT_NADA},
    {672, ROMP_JARRA, CONT_NADA},
    {928, ROMP_BARRIL, CONT_ONIGIRI},
    {1184, ROMP_CAJA, CONT_NADA},
    {1408, ROMP_FAROL, CONT_VIDA},    /* La vida extra, escondida cerca de la salida.      */
};
static const Zona zona_aldea = {1536, 3, 6, oleadas_aldea, rompibles_aldea};

/*--- 2. Bosque de bambú: 1280 px. Rápidos y amarillos; al final, el pesado rojo -------*/
static const Oleada oleadas_bosque[3] =
{
    {256, 3, LADO_ALTERNO, {{R, RANGO_BLANCO}, {R, RANGO_BLANCO}, {B, RANGO_AMARILLO}}, ESC_NADA, ESC_LUCINDA, {0, 0, 0, 0}},
    {576, 3, LADO_ALTERNO, {{B, RANGO_AMARILLO}, {B, RANGO_AMARILLO}, {B, RANGO_AMARILLO}}, ESC_NADA, ESC_FENRIR_GAG, {0, 0, 0, 0}},
    {896, 3, LADO_ALTERNO, {{P, RANGO_ROJO}, {B, RANGO_AMARILLO}, {B, RANGO_AMARILLO}}, ESC_NADA, ESC_NADA, {0, 0, 0, 0}},
};
static const Rompible rompibles_bosque[4] =
{
    {224, ROMP_CAJA, CONT_ONIGIRI},
    {512, ROMP_BARRIL, CONT_SHURIKEN},
    {800, ROMP_CAJA, CONT_ONIGIRI},
    {1056, ROMP_JARRA, CONT_NADA},
};
static const Zona zona_bosque = {1280, 3, 4, oleadas_bosque, rompibles_bosque};

/*--- 3. Monasterio de Rex: 768 px. La escena abre la zona (x = 0); 3 amarillos ----------*/
static const Oleada oleadas_monasterio[1] =
{
    {0, 3, LADO_ALTERNO, {{B, RANGO_AMARILLO}, {B, RANGO_AMARILLO}, {B, RANGO_AMARILLO}}, ESC_REX, ESC_NADA, {0, 0, 0, 0}},
};
static const Rompible rompibles_monasterio[3] =
{
    {200, ROMP_FAROL, CONT_NADA},
    {352, ROMP_JARRA, CONT_PESCADO},
    {480, ROMP_FAROL, CONT_NADA},
};
static const Zona zona_monasterio = {768, 1, 3, oleadas_monasterio, rompibles_monasterio};

/*--- 4. Dojo: 1024 px. La puerta, con verdes; después, Gomasuri-dono, solo ---------------*/
static const Oleada oleadas_dojo[2] =
{
    {256, 3, LADO_ALTERNO, {{B, RANGO_VERDE}, {B, RANGO_VERDE}, {R, RANGO_AMARILLO}}, ESC_NADA, ESC_NADA, {0, 0, 0, 0}},
    {768, 1, LADO_DERECHA, {{J, 0}, {0, 0}, {0, 0}}, ESC_JEFE_ENTRA, ESC_VILLANO_HUYE, {0, 0, 0, 0}},
};
static const Rompible rompibles_dojo[4] =
{
    {128, ROMP_SHOJI, CONT_ONIGIRI},
    {384, ROMP_JARRA, CONT_NADA},
    {576, ROMP_SHOJI, CONT_PESCADO},
    {704, ROMP_JARRA, CONT_NADA},
};
static const Zona zona_dojo = {1024, 2, 4, oleadas_dojo, rompibles_dojo};

#undef B
#undef R
#undef P
#undef J

const Zona *fase1_zona(u8 id)
{
    switch (id)                      /* Un switch y no zonas[id]: Zona no es potencia de 2.  */
    {
    case ZONA_ALDEA:
        return &zona_aldea;
    case ZONA_BOSQUE:
        return &zona_bosque;
    case ZONA_MONASTERIO:
        return &zona_monasterio;
    case ZONA_DOJO:
        return &zona_dojo;
    default:
        return 0;
    }
}

void fase1_aplicar_escena(u8 escena, Apoyos *apoyos)
{
    if (escena == ESC_LUCINDA)
    {
        apoyos_desbloquear(apoyos, APOYO_LUCINDA);     /* Se une en el bosque.             */
    }
    else if (escena == ESC_REX)
    {
        apoyos_desbloquear(apoyos, APOYO_REX);         /* Se une en su monasterio...       */
        apoyos_activar_barra(apoyos);                  /* ...y la barra se activa, llena.  */
    }
}
