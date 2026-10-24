/*---------------------------------------------------------------------------------
    guion.c — Un mando leído de una tabla de tiempo (ver guion.h).

    PROPÓSITO
      Para un cuadro, juntar (OR) los botones de todos los tramos que lo
      contienen; hacer lo mismo con el cuadro anterior; "recién pulsado" es lo
      que está ahora y no estaba antes. Igual que hace el mando real con
      padsCurrent y padsDown.

    SE CONECTA CON
      Incluye:  juego/guion.h.
      Lo usan:  juego/mando.c (GUION_PRUEBA), tests/prueba_recorrido.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Mando
---------------------------------------------------------------------------------*/
#include "juego/guion.h"

/*--- mantenidos: los botones de los tramos que contienen el cuadro ---------------
    Se recorre la tabla con un PUNTERO que avanza de tramo en tramo (t++ suma el
    tamaño de un TramoGuion: 6 bytes, porque el compilador añade uno de relleno).
    Con guion[i], tcc816 calcula i * 6 con su rutina de multiplicar (tcc__mul) en
    cada acceso: 3 por vuelta, entre 26 y 78 multiplicaciones por frame (medido en
    la unidad 14 de aprendizaje/). */
static u8 mantenidos(const TramoGuion *t, u8 n, u16 cuadro)
{
    u8 b = 0;
    for (; n != 0; n--, t++)
    {
        if (cuadro >= t->desde && cuadro < t->hasta)
        {
            b |= t->botones;                         /* Este tramo está activo.             */
        }
    }
    return b;
}

void guion_mando(const TramoGuion *guion, u8 n, u16 cuadro, Mando *m)
{
    u8 ahora = mantenidos(guion, n, cuadro);
    u8 antes = cuadro ? mantenidos(guion, n, (u16)(cuadro - 1)) : 0;
    u8 nuevos = (u8)(ahora & ~antes);                /* Pulsados en este cuadro.            */

    m->izquierda = (u8)((ahora & BOTON_IZQUIERDA) != 0);
    m->derecha = (u8)((ahora & BOTON_DERECHA) != 0);
    m->abajo = (u8)((ahora & BOTON_ABAJO) != 0);
    m->correr = (u8)((ahora & BOTON_CORRER) != 0);
    m->saltar = (u8)((ahora & BOTON_SALTAR) != 0);
    m->saltar_pulsado = (u8)((nuevos & BOTON_SALTAR) != 0);
    m->atacar_pulsado = (u8)((nuevos & BOTON_ATACAR) != 0);
    m->espada_pulsado = (u8)((nuevos & BOTON_ESPADA) != 0);
    m->hud_pulsado = 0;                              /* El guion no toca SELECT ni START.   */
    m->salir_pulsado = 0;
    m->actividad = (u8)(ahora != 0);
}
