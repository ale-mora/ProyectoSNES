/*---------------------------------------------------------------------------------
    prueba_metasprite.c — Pruebas de juego/metasprite.c.

    PROPÓSITO
      Fijar con ejemplos cómo se recorre una tabla metasprite de gfx4snes -T:
      termina en una entrada con dx = metasprite_end (-128, que en un u16 es
      0xFF80) y puede tener MENOS de cuatro piezas, porque gfx4snes omite los
      bloques de 32x32 totalmente transparentes (un Tarou dormido solo ocupa
      la mitad inferior de la celda). El espejo (mirar a la izquierda) debe
      copiar solo las piezas que hay y poner el terminador justo detrás.

    SE CONECTA CON
      Incluye:  minitest.h, juego/metasprite.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_metasprite).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (estáticas de este archivo)
      CUATRO   t_metasprite[]  cuadro completo: 4 piezas + terminador
      DOS      t_metasprite[]  solo la mitad inferior: 2 piezas + terminador
      CERO     t_metasprite[]  cuadro vacío: solo el terminador
      SIN_FIN  t_metasprite[]  tabla rota sin terminador (5 piezas): el límite de 4 protege
      espejo   t_metasprite[]  destino del espejo, con basura antes de cada prueba
    → glosario: Metasprite, Sprite dinámico, Dirección
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/metasprite.h"

#define PROPS 0x30                      /* OBJ_PAL(0) | OBJ_PRIO(3), como genera gfx4snes. */

static const t_metasprite CUATRO[] = {
    {0, 0, 0, PROPS, 0}, {32, 0, 1, PROPS, 0}, {0, 32, 8, PROPS, 0}, {32, 32, 9, PROPS, 0},
    {METASPRITE_FIN, 0, 0, 0, 0}};
static const t_metasprite DOS[] = {
    {0, 32, 8, PROPS, 0}, {32, 32, 9, PROPS, 0},
    {METASPRITE_FIN, 0, 0, 0, 0}};
static const t_metasprite CERO[] = {{METASPRITE_FIN, 0, 0, 0, 0}};
static const t_metasprite SIN_FIN[] = {
    {0, 0, 0, PROPS, 0}, {32, 0, 1, PROPS, 0}, {0, 32, 8, PROPS, 0}, {32, 32, 9, PROPS, 0},
    {0, 0, 2, PROPS, 0}};

static t_metasprite espejo[METASPRITE_PIEZAS_MAX + 1];

/*--- ensuciar: llena el destino de basura para comprobar que todo se escribe ----*/
static void ensuciar(void)
{
    u8 i;
    for (i = 0; i <= METASPRITE_PIEZAS_MAX; i++)
    {
        espejo[i].dx = 0x1234;
        espejo[i].dy = 0x1234;
        espejo[i].dtile = 0x1234;
        espejo[i].props = 0x12;
        espejo[i].unused = 0x12;
    }
}

void pruebas_metasprite(void)
{
    u8 n;

    /* El terminador del SDK, visto como u16. */
    COMPROBAR(METASPRITE_FIN == 0xFF80);
    COMPROBAR((u16)-128 == METASPRITE_FIN);

    /* Contar piezas: para en el terminador, o en 4 si la tabla está rota. */
    COMPROBAR(metasprite_piezas(CUATRO) == 4);
    COMPROBAR(metasprite_piezas(DOS) == 2);
    COMPROBAR(metasprite_piezas(CERO) == 0);
    COMPROBAR(metasprite_piezas(SIN_FIN) == 4);

    /* Espejo de un cuadro completo: cada pieza cambia de lado (dx 0 <-> 32),
       conserva fila y tile, gana el bit de espejo horizontal, y el terminador
       queda en la posición 4. */
    ensuciar();
    n = metasprite_espejar(CUATRO, espejo);
    COMPROBAR(n == 4);
    COMPROBAR(espejo[0].dx == 32 && espejo[1].dx == 0 && espejo[2].dx == 32 && espejo[3].dx == 0);
    COMPROBAR(espejo[0].dy == 0 && espejo[2].dy == 32);
    COMPROBAR(espejo[1].dtile == 1 && espejo[3].dtile == 9);
    COMPROBAR(espejo[0].props == (PROPS | METASPRITE_HFLIP));
    COMPROBAR(espejo[3].props == (PROPS | METASPRITE_HFLIP));
    COMPROBAR(espejo[0].unused == 0);
    COMPROBAR(espejo[4].dx == METASPRITE_FIN);

    /* Espejo de medio cuadro (el caso que gfx4snes -T produce con arte real):
       solo dos piezas y el terminador JUSTO detrás, no en la posición 4. */
    ensuciar();
    n = metasprite_espejar(DOS, espejo);
    COMPROBAR(n == 2);
    COMPROBAR(espejo[0].dx == 32 && espejo[0].dy == 32 && espejo[0].dtile == 8);
    COMPROBAR(espejo[1].dx == 0 && espejo[1].dy == 32 && espejo[1].dtile == 9);
    COMPROBAR(espejo[2].dx == METASPRITE_FIN);

    /* Cuadro vacío: nada que copiar, el terminador va el primero. */
    ensuciar();
    COMPROBAR(metasprite_espejar(CERO, espejo) == 0);
    COMPROBAR(espejo[0].dx == METASPRITE_FIN);

    /* Tabla sin terminador: se copian como mucho 4 piezas y SIEMPRE se escribe
       un terminador, para que el motor del SDK no siga leyendo memoria ajena. */
    ensuciar();
    COMPROBAR(metasprite_espejar(SIN_FIN, espejo) == 4);
    COMPROBAR(espejo[4].dx == METASPRITE_FIN);
}
