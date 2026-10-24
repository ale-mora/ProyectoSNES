/*---------------------------------------------------------------------------------
    oleadas.c — La máquina de estados de las zonas (ver oleadas.h).

    PROPÓSITO
      Dos estados por zona: LIBRE (la cámara avanza hasta el punto de la zona) y
      BLOQUEADA (oleada viva: la cámara no se mueve). Llegar al punto bloquea;
      oleadas_vencer libera y pasa a la zona siguiente. Los puntos son posiciones de la
      cámara (píxeles del nivel) y son PROVISIONALES: se mueven al dibujar la fase 1 en
      Tiled (docs/diseno-beat-em-up.md §7: objetos `oleada`).

    SE CONECTA CON
      Incluye:  juego/oleadas.h.
      Lo usan:  pantallas/pantalla_fase1.c y tests/prueba_oleadas.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (constantes)
      ZONA_CAMARA  u16[OLEADAS_ZONAS]  posición de la cámara en que se bloquea cada zona
    → glosario: Zona, Oleada
---------------------------------------------------------------------------------*/
#include "juego/oleadas.h"

/* static const: se queda en la ROM (una const global se copiaría a la WRAM al arrancar). */
static const u16 ZONA_CAMARA[OLEADAS_ZONAS] = {256, 640, 1024, 1536};

void oleadas_iniciar(Oleadas *o)
{
    o->actual = 0;                                   /* Primera zona, todavía lejos.        */
    o->bloqueada = 0;
    o->libre[0] = 0;
    o->libre[1] = 0;
}

u16 oleadas_limite(const Oleadas *o)
{
    if (o->actual >= OLEADAS_ZONAS)
    {
        return 0xFFFF;                               /* Sin zonas: solo el final del nivel. */
    }
    return ZONA_CAMARA[o->actual];
}

u8 oleadas_actualizar(Oleadas *o, u16 camara)
{
    if (!o->bloqueada && o->actual < OLEADAS_ZONAS && camara >= ZONA_CAMARA[o->actual])
    {
        o->bloqueada = 1;                            /* Llegó: la cámara se para y salen... */
        return 1;                                    /* ...los enemigos de esta oleada.     */
    }
    return 0;
}

void oleadas_vencer(Oleadas *o)
{
    if (o->bloqueada)
    {
        o->bloqueada = 0;                            /* Libre otra vez («GO →»)...          */
        o->actual++;                                 /* ...hasta el punto de la siguiente.  */
    }
}
