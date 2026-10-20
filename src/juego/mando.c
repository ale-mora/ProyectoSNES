/*---------------------------------------------------------------------------------
    mando.c — Lee el mando 1 de la SNES y lo traduce a la struct Mando.

    PROPÓSITO
      Es la única función del juego que mira los bits del pad. PVSnesLib los lee
      en cada VBlank (el NMI del SDK) y los deja en pad_keys (mantenidos) y
      pad_keysdown (recién pulsados); aquí se convierten en intenciones con
      nombre según la asignación de docs/diseno-demo-jugable.md §3, para que la
      lógica pura (tarou.c) no sepa qué botón físico es cada cosa.

    SE CONECTA CON
      Incluye:  <snes.h> (padsCurrent, padsDown, KEY_*), juego/mando.h.
      Lo usan:  pantallas/pantalla_nivel1.c (una vez por frame).

    HARDWARE QUE TOCA
      El mando: a través de padsCurrent()/padsDown() del SDK, que leen los
      registros de autolectura del joypad ($4218/$4219) durante el VBlank.

    VARIABLES (globales y estáticas de este archivo)
      mantenidos  u16  bits KEY_*  botones mantenidos este frame
      pulsados    u16  bits KEY_*  botones que bajaron este frame

    → glosario: Mando, VBlank
---------------------------------------------------------------------------------*/
#include <snes.h>
#include "juego/mando.h"
#ifdef GUION_PRUEBA
#include "juego/guion_tutorial.h"
#endif

static u16 mantenidos;             /* padsCurrent(0): lo que está pulsado ahora.          */
static u16 pulsados;               /* padsDown(0): lo que acaba de bajar en este frame.   */
#ifdef GUION_PRUEBA
static u16 cuadro_guion;           /* Cuadros desde que empezó el nivel (ROM de prueba).  */
#endif

void mando_iniciar(void)
{
#ifdef GUION_PRUEBA
    cuadro_guion = 0;              /* El guion empieza a contar con el nivel.              */
#endif
}

void mando_leer(Mando *m)
{
    mantenidos = padsCurrent(0);
    pulsados = padsDown(0);
#ifdef GUION_PRUEBA
    /* ROM de prueba: los botones del juego salen del guion del recorrido; del mando
       real solo cuentan SELECT (HUD) y START (salir). Ver juego/guion.h. */
    guion_mando(GUION_TUTORIAL, GUION_TUTORIAL_TRAMOS, cuadro_guion, m);
    cuadro_guion++;
    m->hud_pulsado = (u8)((pulsados & KEY_SELECT) != 0);
    m->salir_pulsado = (u8)((pulsados & KEY_START) != 0);
    return;
#endif

    /* Mantenidos: dirección, agacharse, correr y el B que alarga el salto. */
    m->izquierda = (u8)((mantenidos & KEY_LEFT) != 0);
    m->derecha = (u8)((mantenidos & KEY_RIGHT) != 0);
    m->abajo = (u8)((mantenidos & KEY_DOWN) != 0);
    m->correr = (u8)((mantenidos & KEY_Y) != 0);
    m->saltar = (u8)((mantenidos & KEY_B) != 0);

    /* Recién pulsados: acciones de un solo disparo. */
    m->saltar_pulsado = (u8)((pulsados & KEY_B) != 0);
    m->atacar_pulsado = (u8)((pulsados & KEY_A) != 0);
    m->espada_pulsado = (u8)((pulsados & KEY_R) != 0);
    m->hud_pulsado = (u8)((pulsados & KEY_SELECT) != 0);
    m->salir_pulsado = (u8)((pulsados & KEY_START) != 0);

    /* Cualquier cosa tocada cuenta como actividad (reinicia el contador de sueño). */
    m->actividad = (u8)((mantenidos | pulsados) != 0);
}
