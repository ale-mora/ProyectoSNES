/*---------------------------------------------------------------------------------
    guion_tutorial.c — Los tramos del recorrido completo (ver guion_tutorial.h).

    PROPÓSITO
      Una tabla: del cuadro A al B, mantener tales botones. Los cuadros cuentan
      desde que empieza el nivel. Cada línea dice qué obstáculo resuelve; las x
      son las de docs/diseno-tutorial-movimiento.md §2.

    SE CONECTA CON
      Incluye:  juego/guion_tutorial.h.
      Lo usan:  juego/mando.c (GUION_PRUEBA), tests/prueba_recorrido.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (globales)
      GUION_TUTORIAL  const TramoGuion[12]  en la ROM (const)
    → glosario: Mando
---------------------------------------------------------------------------------*/
#include "juego/guion_tutorial.h"

const TramoGuion GUION_TUTORIAL[GUION_TUTORIAL_TRAMOS] = {
    {0, 1725, BOTON_DERECHA},        /* Caminar a la derecha hasta el tronco.            */
    {235, 265, BOTON_SALTAR},        /* Pegado al escalón de 16 px (x 260): salto.       */
    {300, 330, BOTON_SALTAR},        /* Contra el de 32 px (x 308): otro salto.          */
    {560, 600, BOTON_SALTAR},        /* Pegado al muro de 48 px (x 516): B mantenido.    */
    {860, 1220, BOTON_ABAJO},        /* Túnel (boca en x 772): agachado hasta salir.     */
    {1220, 1350, BOTON_CORRER},      /* Y: correr hacia el foso...                       */
    {1288, 1320, BOTON_SALTAR},      /* ...y saltar en x 1080, con el pie aún en tierra. */
    {1412, 1445, BOTON_SALTAR},      /* Antes de las zarzas: a la primera plataforma.    */
    {1572, 1605, BOTON_SALTAR},      /* Final de la segunda: salto por encima del resto. */
    {1740, 1745, BOTON_ESPADA},      /* Contra el tronco (x 1564): R, desenvainar...     */
    {1840, 1845, BOTON_ATACAR},      /* ...y A: el golpe rompe el tronco.                */
    {1880, 2600, BOTON_DERECHA},     /* Hasta la salida (x 1920).                        */
};
