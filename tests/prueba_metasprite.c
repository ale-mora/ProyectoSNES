/*---------------------------------------------------------------------------------
    prueba_metasprite.c — Pruebas de juego/metasprite.c.

    PROPÓSITO
      Fijar con ejemplos cómo se recorre una tabla metasprite de gfx4snes -T:
      termina en una entrada con dx = metasprite_end (-128, que en un u16 es
      0xFF80) y puede tener MENOS de seis piezas (celda de 96x64 = 3x2 piezas
      de 32x32), porque gfx4snes omite los bloques totalmente transparentes
      (un Tarou dormido solo ocupa la mitad inferior de la celda). El espejo
      (mirar a la izquierda) debe copiar solo las piezas que hay, llevar cada
      una a 64 - dx y poner el terminador justo detrás.

    SE CONECTA CON
      Incluye:  minitest.h, juego/metasprite.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_metasprite).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (estáticas de este archivo)
      SEIS     t_metasprite[]  cuadro completo de 96x64: 6 piezas + terminador
      TRES     t_metasprite[]  solo la fila de abajo: 3 piezas + terminador
      CERO     t_metasprite[]  cuadro vacío: solo el terminador
      SIN_FIN  t_metasprite[]  tabla rota sin terminador (7 piezas): el límite de 6 protege
      espejo   t_metasprite[]  destino del espejo, con basura antes de cada prueba
    → glosario: Metasprite, Sprite dinámico, Dirección
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/metasprite.h"

#define PROPS 0x30                      /* OBJ_PAL(0) | OBJ_PRIO(3), como genera gfx4snes. */

static const t_metasprite SEIS[] = {
    {0, 0, 0, PROPS, 0}, {32, 0, 1, PROPS, 0}, {64, 0, 2, PROPS, 0},
    {0, 32, 6, PROPS, 0}, {32, 32, 7, PROPS, 0}, {64, 32, 8, PROPS, 0},
    {METASPRITE_FIN, 0, 0, 0, 0}};
static const t_metasprite TRES[] = {
    {0, 32, 6, PROPS, 0}, {32, 32, 7, PROPS, 0}, {64, 32, 8, PROPS, 0},
    {METASPRITE_FIN, 0, 0, 0, 0}};
static const t_metasprite CERO[] = {{METASPRITE_FIN, 0, 0, 0, 0}};
static const t_metasprite SIN_FIN[] = {
    {0, 0, 0, PROPS, 0}, {32, 0, 1, PROPS, 0}, {64, 0, 2, PROPS, 0},
    {0, 32, 6, PROPS, 0}, {32, 32, 7, PROPS, 0}, {64, 32, 8, PROPS, 0},
    {0, 0, 3, PROPS, 0}};

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

    /* El terminador del SDK, visto como u16, y la geometría de la celda de 96x64. */
    COMPROBAR(METASPRITE_FIN == 0xFF80);
    COMPROBAR((u16)-128 == METASPRITE_FIN);
    COMPROBAR(METASPRITE_PIEZAS_MAX == 6);
    COMPROBAR(METASPRITE_ESPEJO_DX == 64);         /* 96 - 32: la última columna de piezas. */

    /* Contar piezas: para en el terminador, o en 6 si la tabla está rota. */
    COMPROBAR(metasprite_piezas(SEIS) == 6);
    COMPROBAR(metasprite_piezas(TRES) == 3);
    COMPROBAR(metasprite_piezas(CERO) == 0);
    COMPROBAR(metasprite_piezas(SIN_FIN) == 6);

    /* Espejo de un cuadro completo: cada pieza pasa a 64 - dx (0 <-> 64, 32 se
       queda), conserva fila y bloque, gana el bit de espejo horizontal, y el
       terminador queda en la posición 6. */
    ensuciar();
    n = metasprite_espejar(SEIS, espejo);
    COMPROBAR(n == 6);
    COMPROBAR(espejo[0].dx == 64 && espejo[1].dx == 32 && espejo[2].dx == 0);
    COMPROBAR(espejo[3].dx == 64 && espejo[4].dx == 32 && espejo[5].dx == 0);
    COMPROBAR(espejo[0].dy == 0 && espejo[3].dy == 32);
    COMPROBAR(espejo[2].dtile == 2 && espejo[5].dtile == 8);
    COMPROBAR(espejo[0].props == (PROPS | METASPRITE_HFLIP));
    COMPROBAR(espejo[5].props == (PROPS | METASPRITE_HFLIP));
    COMPROBAR(espejo[0].unused == 0);
    COMPROBAR(espejo[6].dx == METASPRITE_FIN);

    /* Espejo de media celda (el caso que gfx4snes -T produce con un Tarou
       tumbado): solo tres piezas y el terminador JUSTO detrás. */
    ensuciar();
    n = metasprite_espejar(TRES, espejo);
    COMPROBAR(n == 3);
    COMPROBAR(espejo[0].dx == 64 && espejo[0].dy == 32 && espejo[0].dtile == 6);
    COMPROBAR(espejo[2].dx == 0 && espejo[2].dtile == 8);
    COMPROBAR(espejo[3].dx == METASPRITE_FIN);

    /* Cuadro vacío: nada que copiar, el terminador va el primero. */
    ensuciar();
    COMPROBAR(metasprite_espejar(CERO, espejo) == 0);
    COMPROBAR(espejo[0].dx == METASPRITE_FIN);

    /* Tabla sin terminador: se copian como mucho 6 piezas y SIEMPRE se escribe
       un terminador, para que el motor del SDK no siga leyendo memoria ajena. */
    ensuciar();
    COMPROBAR(metasprite_espejar(SIN_FIN, espejo) == 6);
    COMPROBAR(espejo[6].dx == METASPRITE_FIN);
}
