/*---------------------------------------------------------------------------------
    mando.h — El mando visto por la lógica del juego: qué quiere hacer el jugador
    este frame, sin saber de bits ni de registros.

    PROPÓSITO
      La lógica pura (tarou.c) no debe leer padsCurrent()/padsDown(): si lo
      hiciera no se podría probar en el PC. Esta struct traduce el mando a
      intenciones con nombre (izquierda, saltar_pulsado...). mando_leer() la
      rellena en la SNES (juego/mando.c); las pruebas la rellenan a mano.
      Asignación de botones (docs/diseno-demo-jugable.md §3): cruceta mueve,
      Abajo agacha, Y corre, B salta, A ataca, R envaina/desenvaina, SELECT
      alterna el HUD de depuración y START vuelve al título.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Lo usan:  juego/tarou.c (lee la struct), juego/mando.c (la rellena),
                pantallas/pantalla_nivel1.c (START y SELECT), tests/.

    HARDWARE QUE TOCA
      ninguno aquí; los puertos del mando los lee mando.c a través del SDK.

    VARIABLES (campos de Mando; todos valen 0 o 1)
      izquierda, derecha  mantenidos  cruceta
      arriba              mantenido   cruceta (profundidad del beat 'em up)
      abajo               mantenido   agacharse
      correr              mantenido   Y
      saltar              mantenido   B (alarga el salto mientras se mantiene)
      saltar_pulsado      recién      B bajó en este frame
      atacar_pulsado      recién      A
      espada_pulsado      recién      R
      hud_pulsado         recién      SELECT
      salir_pulsado       recién      START
      actividad           cualquier botón o dirección mantenido o pulsado (para dormir)

    → glosario: Mando, struct
---------------------------------------------------------------------------------*/
#ifndef MANDO_H
#define MANDO_H

#include "sistema/tipos.h"

typedef struct
{
    u8 izquierda, derecha, arriba, abajo, correr, saltar;                        /* mantenidos     */
    u8 saltar_pulsado, atacar_pulsado, espada_pulsado, hud_pulsado, salir_pulsado; /* recién pulsados*/
    u8 actividad;                                                                /* algo tocado    */
} Mando;

/* Rellena `m` leyendo el mando 1 de la SNES. Solo existe en la ROM (juego/mando.c). */
void mando_leer(Mando *m);

/* Al entrar en el nivel. En el juego normal no hace nada; en la ROM de prueba
   (compilada con -DGUION_PRUEBA) pone a cero el reloj del guion del recorrido. */
void mando_iniciar(void);

#endif /* MANDO_H */
