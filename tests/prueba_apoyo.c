/*---------------------------------------------------------------------------------
    prueba_apoyo.c — Pruebas de juego/apoyo.c, juego/dialogo.c y de los «personajes» de
    los apoyos y las tiras nuevas de Tarou (juego/personaje.c).

    Comprueban: la barra (sube al conectar, no pasa del máximo, solo si hay apoyo), R cambia
    el apoyo, X solo con la barra llena, la máquina ENTRA → ATACA → SALE → FUERA con sus
    frames, los impactos (Lucinda 3, el último derriba; Rex y Fenrir 1), la escena de texto
    (A no pasa antes de DIALOGO_MIN, pasa sola a los DIALOGO_MAX, acaba tras la última) y
    las celdas y tablas de los apoyos y de Tarou.
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/apoyo.h"
#include "juego/dialogo.h"
#include "juego/personaje.h"

static void barra_y_cambio(void)
{
    Apoyo a;
    u8 k;

    apoyo_iniciar(&a, 0);                            /* Antes del monasterio: no carga.     */
    apoyo_cargar(&a);
    COMPROBAR(a.barra == 0 && !apoyo_listo(&a));
    COMPROBAR(apoyo_invocar(&a, 100, 200, 0) == 0);

    apoyo_iniciar(&a, 1);
    for (k = 0; k < 15; k++)
    {
        apoyo_cargar(&a);
    }
    COMPROBAR(a.barra == 30 && !apoyo_listo(&a));
    COMPROBAR(apoyo_invocar(&a, 100, 200, 0) == 0); /* Sin llenar: X no hace nada.         */
    for (k = 0; k < 5; k++)
    {
        apoyo_cargar(&a);
    }
    COMPROBAR(a.barra == APOYO_BARRA_MAX && apoyo_listo(&a));   /* No pasa del máximo.  */

    COMPROBAR(a.elegido == APOYO_LUCINDA);
    apoyo_cambiar(&a);
    COMPROBAR(a.elegido == APOYO_REX);
    apoyo_cambiar(&a);
    apoyo_cambiar(&a);
    COMPROBAR(a.elegido == APOYO_LUCINDA);           /* Da la vuelta.                       */
}

/* Cuenta impactos y frames de un ataque entero del apoyo `quien`. */
static void ataque_completo(u8 quien, u8 impactos_esperados)
{
    Apoyo a;
    u16 frames = 0;
    u8 impactos = 0, derribos = 0, k;

    apoyo_iniciar(&a, 1);
    a.elegido = quien;
    for (k = 0; k < 16; k++)
    {
        apoyo_cargar(&a);
    }
    COMPROBAR(apoyo_invocar(&a, 200, 190, 0) == 1);
    COMPROBAR(a.barra == 0 && apoyo_activo(&a));     /* Gasta la barra.                     */
    COMPROBAR(a.x == 200 - APOYO_ENTRA_FRAMES * APOYO_ENTRA_PASO);   /* Llega desde atrás.  */
    apoyo_cargar(&a);
    COMPROBAR(a.barra == 0);                         /* En pantalla no carga.               */
    apoyo_cambiar(&a);
    COMPROBAR(a.elegido == quien);                   /* Ni se cambia.                       */
    while (apoyo_activo(&a) && frames < 500)
    {
        apoyo_actualizar(&a);
        frames++;
        if (a.evento == APOYO_EVENTO_IMPACTO)
        {
            impactos++;
            if (apoyo_derriba(&a))
            {
                derribos++;
            }
        }
        if (frames == APOYO_ENTRA_FRAMES)
        {
            COMPROBAR(a.x == 200 && a.estado == APOYO_ATACA);   /* Llegó a su sitio.        */
        }
    }
    COMPROBAR(frames == APOYO_ENTRA_FRAMES + APOYO_ATAQUE_FRAMES + APOYO_SALE_FRAMES);
    COMPROBAR(impactos == impactos_esperados);
    COMPROBAR(derribos == 1);                        /* Lucinda: solo el último tajo.       */
    COMPROBAR(!apoyo_activo(&a));
}

static void mirando_izquierda(void)
{
    Apoyo a;
    u8 k;

    apoyo_iniciar(&a, 1);
    for (k = 0; k < 16; k++)
    {
        apoyo_cargar(&a);
    }
    COMPROBAR(apoyo_invocar(&a, 300, 190, 1) == 1);
    COMPROBAR(a.x == 300 + APOYO_ENTRA_FRAMES * APOYO_ENTRA_PASO);   /* Viene por la derecha. */
    for (k = 0; k < APOYO_ENTRA_FRAMES; k++)
    {
        apoyo_actualizar(&a);
    }
    COMPROBAR(a.x == 300);
}

static void cuadros(void)
{
    Apoyo a;

    apoyo_iniciar(&a, 1);
    a.estado = APOYO_ATACA;
    a.reloj = 0;
    COMPROBAR(apoyo_cuadro(&a) == 0 && apoyo_atacando(&a));
    a.reloj = 13;
    COMPROBAR(apoyo_cuadro(&a) == 2);
    a.reloj = 36;
    COMPROBAR(apoyo_cuadro(&a) == 5);                /* Se queda en el último.              */
    a.estado = APOYO_ENTRA;
    a.reloj = 5;
    COMPROBAR(apoyo_cuadro(&a) == 1 && !apoyo_atacando(&a));
}

static void dialogo(void)
{
    static const char *const L[4] = {"A", "B", "C", "D"};
    Dialogo d;
    u16 k;

    dialogo_iniciar(&d);
    COMPROBAR(!d.activo && dialogo_actualizar(&d, 1) == 0);
    dialogo_empezar(&d, L, 2);
    COMPROBAR(d.activo && dialogo_linea(&d, 0)[0] == 'A' && dialogo_linea(&d, 1)[0] == 'B');
    dialogo_actualizar(&d, 1);                       /* A pulsado de antes: no se la salta. */
    COMPROBAR(d.pagina == 0);
    for (k = 1; k < DIALOGO_MIN - 1; k++)
    {
        dialogo_actualizar(&d, 0);
    }
    COMPROBAR(dialogo_actualizar(&d, 1) == 1);       /* Ya puede: página 2.                 */
    COMPROBAR(d.pagina == 1 && dialogo_linea(&d, 0)[0] == 'C');
    for (k = 0; k < DIALOGO_MAX - 1; k++)
    {
        dialogo_actualizar(&d, 0);
    }
    COMPROBAR(d.activo);
    dialogo_actualizar(&d, 0);                       /* Pasa sola: era la última.           */
    COMPROBAR(!d.activo);
}

static void personajes(void)
{
    const AnimProta *t;

    COMPROBAR(personaje_ancho(PERSONAJE_LUCINDA) == 96 && personaje_alto(PERSONAJE_LUCINDA) == 96);
    COMPROBAR(personaje_ancho(PERSONAJE_FENRIR) == 112 && personaje_alto(PERSONAJE_FENRIR) == 112);
    COMPROBAR(personaje_ancho(PERSONAJE_TAROU) == 96 && personaje_alto(PERSONAJE_TAROU) == 64);
    t = personaje_anims(PERSONAJE_REX);
    COMPROBAR(t[ANIM_CORRER].tira == APOYO_TIRA_ENTRADA && t[ANIM_CORRER].bucle);
    COMPROBAR(t[ANIM_ESPECIAL].tira == APOYO_TIRA_ATAQUE && t[ANIM_ESPECIAL].cuadros == 6);
    COMPROBAR(t[ANIM_ESPECIAL].cuadros * t[ANIM_ESPECIAL].frames == APOYO_ATAQUE_FRAMES);
    t = personaje_anims(PERSONAJE_TAROU);            /* Punto 8: las tiras nuevas de Tarou. */
    COMPROBAR(t[ANIM_GOLPE_4].tira == TAROU_TIRA_GOLPE_4);
    COMPROBAR(t[ANIM_DERRIBADO].tira == TAROU_TIRA_DERRIBADO);
    COMPROBAR(t[ANIM_LEVANTARSE].tira == TAROU_TIRA_LEVANTARSE);
    COMPROBAR(t[ANIM_VICTORIA].tira == TAROU_TIRA_VICTORIA);
}

void pruebas_apoyo(void)
{
    barra_y_cambio();
    ataque_completo(APOYO_LUCINDA, 3);
    ataque_completo(APOYO_REX, 1);
    ataque_completo(APOYO_FENRIR, 1);
    mirando_izquierda();
    cuadros();
    dialogo();
    personajes();
}
