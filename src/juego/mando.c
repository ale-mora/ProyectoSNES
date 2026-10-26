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
   se mantienen. Va a la derecha hasta que la cámara se bloquea, se vuelve hacia el ninja que
   sale y le da el combo (pulsa A cada 10 cuadros: encadena los 3 golpes), y repite en la
   zona siguiente con dos ninjas, uno por cada lado. */
#define G_IZQ 1
#define G_DER 2
#define G_ARR 4
#define G_ABA 8
#define G_SAL 16
#define G_ATK 32
typedef struct
{
    u16 hasta;
    u8 teclas;
} TramoFase1;
static const TramoFase1 GUION_F1[] = {
    {30, 0},  /* Quieto medio segundo. */
    {200, G_DER},  /* Derecha: la cámara sigue y se bloquea en 256; sale el 1.er ninja por la izquierda. */
    {208, G_IZQ},  /* Se vuelve hacia él (a la izquierda). */
    {209, 0},
    {210, G_ATK},
    {218, 0},
    {219, 0},
    {220, G_ATK},
    {228, 0},
    {229, 0},
    {230, G_ATK},
    {238, 0},
    {239, 0},
    {240, G_ATK},
    {248, 0},
    {249, 0},
    {250, G_ATK},
    {258, 0},
    {259, 0},
    {260, G_ATK},
    {268, 0},
    {269, 0},
    {270, G_ATK},
    {278, 0},
    {279, 0},
    {280, G_ATK},
    {288, 0},
    {289, 0},
    {290, G_ATK},
    {298, 0},
    {299, 0},
    {300, G_ATK},
    {308, 0},
    {309, 0},
    {310, G_ATK},
    {318, 0},
    {319, 0},
    {320, G_ATK},
    {328, 0},
    {329, 0},
    {330, G_ATK},
    {338, 0},
    {339, 0},
    {340, G_ATK},
    {348, 0},
    {349, 0},
    {350, G_ATK},
    {358, 0},
    {359, 0},
    {360, G_ATK},
    {368, 0},
    {369, 0},
    {370, G_ATK},
    {378, 0},
    {379, 0},
    {380, G_ATK},
    {388, 0},
    {389, 0},
    {390, G_ATK},
    {398, 0},
    {399, 0},
    {400, G_ATK},
    {408, 0},
    {409, 0},
    {410, G_ATK},
    {418, 0},
    {419, 0},
    {420, G_ATK},
    {428, 0},
    {429, 0},
    {430, G_ATK},
    {438, 0},
    {439, 0},
    {440, G_ATK},
    {448, 0},
    {449, 0},
    {450, G_ATK},
    {458, 0},
    {459, 0},
    {460, G_ATK},
    {468, 0},
    {469, 0},
    {470, G_ATK},
    {478, 0},
    {479, 0},
    {480, G_ATK},
    {488, 0},
    {489, 0},
    {490, G_ATK},
    {498, 0},
    {499, 0},
    {500, G_ATK},
    {508, 0},
    {509, 0},
    {510, G_ATK},
    {518, 0},
    {520, 0},
    {780, G_DER},  /* Vencida la oleada, derecha hasta la zona 1 (cámara 640): salen 2 ninjas. */
    {781, 0},
    {782, G_ATK},
    {790, 0},
    {791, 0},
    {792, G_ATK},
    {800, 0},
    {801, 0},
    {802, G_ATK},
    {810, 0},
    {811, 0},
    {812, G_ATK},
    {820, 0},
    {821, 0},
    {822, G_ATK},
    {830, 0},
    {831, 0},
    {832, G_ATK},
    {840, 0},
    {841, 0},
    {842, G_ATK},
    {850, 0},
    {851, 0},
    {852, G_ATK},
    {860, 0},
    {861, 0},
    {862, G_ATK},
    {870, 0},
    {871, 0},
    {872, G_ATK},
    {880, 0},
    {881, 0},
    {882, G_ATK},
    {890, 0},
    {891, 0},
    {892, G_ATK},
    {900, 0},
    {901, 0},
    {902, G_ATK},
    {910, 0},
    {911, 0},
    {912, G_ATK},
    {920, 0},
    {921, 0},
    {922, G_ATK},
    {930, 0},
    {931, 0},
    {932, G_ATK},
    {940, 0},
    {941, 0},
    {942, G_ATK},
    {950, 0},
    {951, 0},
    {952, G_ATK},
    {960, 0},
    {961, 0},
    {962, G_ATK},
    {970, 0},
    {971, 0},
    {972, G_ATK},
    {980, 0},
    {981, 0},
    {982, G_ATK},
    {990, 0},
    {991, 0},
    {992, G_ATK},
    {1000, 0},
    {1008, G_IZQ},  /* Se vuelve hacia el ninja que viene por la izquierda. */
    {1009, 0},
    {1010, G_ATK},
    {1018, 0},
    {1019, 0},
    {1020, G_ATK},
    {1028, 0},
    {1029, 0},
    {1030, G_ATK},
    {1038, 0},
    {1039, 0},
    {1040, G_ATK},
    {1048, 0},
    {1049, 0},
    {1050, G_ATK},
    {1058, 0},
    {1059, 0},
    {1060, G_ATK},
    {1068, 0},
    {1069, 0},
    {1070, G_ATK},
    {1078, 0},
    {1079, 0},
    {1080, G_ATK},
    {1088, 0},
    {1089, 0},
    {1090, G_ATK},
    {1098, 0},
    {1099, 0},
    {1100, G_ATK},
    {1108, 0},
    {1109, 0},
    {1110, G_ATK},
    {1118, 0},
    {1119, 0},
    {1120, G_ATK},
    {1128, 0},
    {1129, 0},
    {1130, G_ATK},
    {1138, 0},
    {1139, 0},
    {1140, G_ATK},
    {1148, 0},
    {1149, 0},
    {1150, G_ATK},
    {1158, 0},
    {1159, 0},
    {1160, G_ATK},
    {1168, 0},
    {1169, 0},
    {1170, G_ATK},
    {1178, 0},
    {1179, 0},
    {1180, G_ATK},
    {1188, 0},
    {1189, 0},
    {1190, G_ATK},
    {1198, 0},
    {1199, 0},
    {1200, G_ATK},
    {1208, 0},
    {1209, 0},
    {1210, G_ATK},
    {1218, 0},
    {1219, 0},
    {1220, G_ATK},
    {1228, 0},
    {1229, 0},
    {1230, G_ATK},
    {1238, 0},
    {1239, 0},
    {1240, G_ATK},
    {1248, 0},
    {1249, 0},
    {1250, G_ATK},
    {1258, 0},
    {1259, 0},
    {1260, G_ATK},
    {1268, 0},
    {1269, 0},
    {1270, G_ATK},
    {1278, 0},
    {1279, 0},
    {1280, G_ATK},
    {1288, 0},
    {1289, 0},
    {1290, G_ATK},
    {1298, 0},
    {1300, 0},
    {1320, 0},
};
#define GUION_F1_TRAMOS (sizeof(GUION_F1) / sizeof(GUION_F1[0]))
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
        m->atacar_pulsado = (u8)((teclas & G_ATK) != 0 && !(teclas_antes & G_ATK));
        m->correr = m->espada_pulsado = 0;
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
