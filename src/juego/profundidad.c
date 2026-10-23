/*---------------------------------------------------------------------------------
    profundidad.c — Orden de dibujo por profundidad (ver profundidad.h).

    PROPÓSITO
      Ordenación por inserción de los índices de entidades, de la y de los pies
      mayor a la menor. Para cada entidad, desde la segunda: se guarda, y las
      que tiene delante con la y MENOR (estrictamente: así es estable) se corren
      un sitio a la derecha hasta encontrar su hueco.

    SE CONECTA CON
      Incluye:  juego/profundidad.h.
      Lo usan:  pantallas/prueba_carga.c y tests/prueba_profundidad.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna (todo local).
    → glosario: Orden por profundidad
---------------------------------------------------------------------------------*/
#include "juego/profundidad.h"

void profundidad_ordenar(u8 *orden, const u16 *y, u8 n)
{
    u8 i, j;
    u8 actual;                                       /* La entidad que se está colocando.   */
    u16 y_actual;                                    /* Su y, leída una sola vez.           */

    for (i = 1; i < n; i++)
    {
        actual = orden[i];
        y_actual = y[actual];
        j = i;
        while (j > 0 && y[orden[j - 1]] < y_actual)  /* La de delante está más arriba:      */
        {
            orden[j] = orden[j - 1];                 /* se corre un sitio a la derecha.     */
            j--;
        }
        orden[j] = actual;                           /* Su hueco.                           */
    }
}
