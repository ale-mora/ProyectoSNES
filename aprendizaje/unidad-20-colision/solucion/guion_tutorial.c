/*---------------------------------------------------------------------------------
    guion_tutorial.c — Los tramos del recorrido completo (ver guion_tutorial.h).

    PROPÓSITO
      Una tabla: del cuadro A al B, mantener tales botones. Los cuadros cuentan
      desde que empieza el nivel. Cada línea dice qué obstáculo resuelve; las x
      son las de docs/diseno-tutorial-movimiento.md §2, con el nivel tal como
      lo retocó el autor en Tiled el 2026-10-20 (inicio en x 56, hueco de
      32 px entre las dos plataformas, zarzas en el fondo del foso).

    SE CONECTA CON
      Incluye:  juego/guion_tutorial.h.
      Lo usan:  juego/mando.c (GUION_PRUEBA), tests/prueba_recorrido.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (globales)
      GUION_TUTORIAL  const TramoGuion[14]  en la ROM (const)
    → glosario: Mando
---------------------------------------------------------------------------------*/
#include "juego/guion_tutorial.h"

const TramoGuion GUION_TUTORIAL[GUION_TUTORIAL_TRAMOS] = {
    {0, 1757, BOTON_DERECHA},        /* Caminar a la derecha hasta el tronco.            */
    {267, 297, BOTON_SALTAR},        /* Pegado al escalón de 16 px (x 260): salto.       */
    {332, 362, BOTON_SALTAR},        /* Contra el de 32 px (x 308): otro salto.          */
    {592, 632, BOTON_SALTAR},        /* Pegado al muro de 48 px (x 516): B mantenido.    */
    {892, 1252, BOTON_ABAJO},        /* Túnel (boca en x 772): agachado hasta salir.     */
    {1252, 1382, BOTON_CORRER},      /* Y: correr hacia el foso...                       */
    {1320, 1352, BOTON_SALTAR},      /* ...y saltar en x 1080, con el pie aún en tierra. */
    {1444, 1477, BOTON_SALTAR},      /* Antes de las zarzas: a la primera plataforma.    */
    {1538, 1560, BOTON_SALTAR},      /* Salto corto a la segunda (hueco de 32 px).       */
    {1606, 1640, BOTON_SALTAR},      /* Final de la segunda: salto por encima del resto. */
    {1772, 1777, BOTON_ESPADA},      /* Contra el tronco (x 1564): R, desenvainar...     */
    {1872, 1877, BOTON_ATACAR},      /* ...y A: el golpe rompe el tronco.                */
    {1912, 2700, BOTON_DERECHA},     /* Hasta la salida (x 1920).                        */
    {2020, 2050, BOTON_SALTAR},      /* Segundo foso (x 1712..1775): salto caminando.    */
};
