/*---------------------------------------------------------------------------------
    prueba_puntofijo.c — Pruebas de sistema/puntofijo.h (formato 12.4) y de los
    tipos portables de sistema/tipos.h.

    PROPÓSITO
      Fijar con ejemplos qué significa el punto fijo 12.4: PF(1) es 16, la parte
      entera se obtiene desplazando 4 bits, y restar más de lo que hay da un
      valor negativo en s16 (las velocidades sí pueden ser negativas).

    SE CONECTA CON
      Incluye:  minitest.h, sistema/puntofijo.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_puntofijo).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Punto fijo, s16
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "sistema/puntofijo.h"

void pruebas_puntofijo(void)
{
    s16 x;

    COMPROBAR(PF(1) == 16);                       /* 1,0 px = 16 dieciseisavos.          */
    COMPROBAR(PF(448) == 7168);                   /* El borde derecho del nivel cabe.     */
    COMPROBAR(PF_ENTERO(PF(100) + 15) == 100);    /* 100 + 15/16 sigue siendo el píxel 100.*/
    COMPROBAR(PF_ENTERO(PF(100) + 16) == 101);    /* 100 + 16/16 = 101.                   */

    x = (s16)(PF(0) - 2);                         /* Restar por debajo de 0...            */
    COMPROBAR(x < 0);                             /* ...da negativo: s16 tiene signo.     */
}
