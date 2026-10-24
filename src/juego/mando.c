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
#if defined(GUION_PRUEBA) || defined(GUION_FASE1)
static u16 cuadro_guion;           /* Cuadros desde que empezó el nivel (ROM de prueba).  */
#endif

#ifdef GUION_FASE1
/* ROM de prueba de la fase 1 del beat 'em up (make con -DGUION_FASE1): el mando sale de
   esta tabla. Cada tramo vale hasta el cuadro `hasta` (exclusive) y dice qué teclas
   se mantienen. Recorre el carril, salta quieto, salta andando y cambia de carril en el aire. */
#define G_IZQ 1
#define G_DER 2
#define G_ARR 4
#define G_ABA 8
#define G_SAL 16
typedef struct
{
    u16 hasta;
    u8 teclas;
} TramoFase1;
static const TramoFase1 GUION_F1[] = {
    {60, 0},                       /* Quieto 1 s: se ve el cuadro de reposo.               */
    {120, G_DER},                  /* Derecha 1 s.                                         */
    {150, G_ABA},                  /* Abajo: hasta el borde del carril.                    */
    {210, G_ARR},                  /* Arriba: hasta el otro borde.                         */
    {240, 0},
    {243, G_SAL},                  /* Salto quieto (3 cuadros de B).                       */
    {300, 0},                      /* Aterriza.                                            */
    {360, G_IZQ},                  /* Izquierda: x se acerca al borde izquierdo.           */
    {363, G_IZQ | G_SAL},          /* Salto andando hacia la izquierda...                  */
    {375, G_IZQ | G_ARR},          /* ...y cambia de carril en el aire (arriba).           */
    {420, 0},
    {480, G_DER | G_ABA},          /* Diagonal: x e y a la vez.                            */
    {540, 0},
};
#define GUION_F1_TRAMOS 13
static u8 teclas_antes;            /* Teclas del cuadro anterior (para el «recién pulsado»). */
#endif

void mando_iniciar(void)
{
#if defined(GUION_PRUEBA) || defined(GUION_FASE1)
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

#ifdef GUION_FASE1
    {
        u8 teclas = 0;
        u8 i;

        for (i = 0; i < GUION_F1_TRAMOS; i++)        /* El primer tramo que aún no acabó.   */
        {
            if (cuadro_guion < GUION_F1[i].hasta)
            {
                teclas = GUION_F1[i].teclas;
                break;
            }
        }
        cuadro_guion++;
        m->izquierda = (u8)((teclas & G_IZQ) != 0);
        m->derecha = (u8)((teclas & G_DER) != 0);
        m->arriba = (u8)((teclas & G_ARR) != 0);
        m->abajo = (u8)((teclas & G_ABA) != 0);
        m->saltar = (u8)((teclas & G_SAL) != 0);
        m->saltar_pulsado = (u8)(m->saltar && !(teclas_antes & G_SAL));
        m->correr = m->atacar_pulsado = m->espada_pulsado = 0;
        m->hud_pulsado = (u8)((pulsados & KEY_SELECT) != 0);   /* Estos dos, del mando real. */
        m->salir_pulsado = (u8)((pulsados & KEY_START) != 0);
        m->actividad = (u8)(teclas != 0);
        teclas_antes = teclas;
        return;
    }
#endif

    /* Mantenidos: dirección, agacharse, correr y el B que alarga el salto. */
    m->izquierda = (u8)((mantenidos & KEY_LEFT) != 0);
    m->derecha = (u8)((mantenidos & KEY_RIGHT) != 0);
    m->abajo = (u8)((mantenidos & KEY_DOWN) != 0);
    m->arriba = (u8)((mantenidos & KEY_UP) != 0);    /* El carril del beat 'em up.          */
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
