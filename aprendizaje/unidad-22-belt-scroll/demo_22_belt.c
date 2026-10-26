/*---------------------------------------------------------------------------------
    demo_22_belt.c — Demo de la unidad 22: el carril (x, y, z), el orden de dibujo
    por profundidad y la cámara con tope, con el CÓDIGO REAL del juego.

    PROPÓSITO
      Compila en el PC (-DPRUEBAS_PC) src/juego/mundo.c, profundidad.c, camara.c y
      oleadas.c y los usa tal cual. Cuatro partes:
        1. El salto en z: un Cuerpo real salta (B) y la demo imprime, frame a frame,
           z (12.4), la altura en píxeles, vz, la y de los pies (no cambia), la y de
           pantalla del cuerpo (y - z) y la de la sombra (no sube).
        2. El orden por profundidad: la tabla de 8 entidades de la fase 1 (Tarou, su
           sombra, tres ninjas, tres sombras) con sus y de pies; Tarou baja por el
           carril cruzándose con los ninjas y la demo imprime el orden de la OAM cuadro
           a cuadro y cuánto cuesta ordenarlo (comparaciones y desplazamientos).
        3. Quién tapa a quién: un dibujo ASCII donde gana, en cada punto, la entrada
           MÁS BAJA de la OAM. Con la tabla ordenada y sin ordenar.
        4. La cámara: Tarou camina a la derecha con el orden del frame de
           pantalla_fase1.c; la cámara solo avanza (2 px/frame), se bloquea en el
           límite de la zona 0 (256) y sigue cuando se vence la oleada.
      Lo que la demo SÍ copia del juego (porque está dentro de un .c con hardware, no se
      puede compilar en el PC): SOMBRA_ALTO (10) y la fórmula de y0 del cuerpo, de
      protagonista_sprite.c. Los tamaños de pieza de la parte 2 son inventados (marcados).

    SE CONECTA CON
      Incluye:  <stdio.h>, <string.h>, juego/mundo.h, juego/profundidad.h,
                juego/camara.h, juego/oleadas.h, juego/nivel.h (ALTO_CELDA).
      Compilar (desde aprendizaje/unidad-22-belt-scroll/):
        gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../src -o demo_22.exe
            demo_22_belt.c ../../src/juego/mundo.c ../../src/juego/profundidad.c
            ../../src/juego/camara.c ../../src/juego/oleadas.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      c, m        Cuerpo, Mando   el protagonista y lo que "pulsa" la demo
      ent_y       u16[8]          y de los pies de cada entidad (sprites_y del juego)
      orden       u8[8]           orden de dibujo (sprites_orden del juego)
    → glosario: Carril, Profundidad (y) y altura (z), Orden por profundidad, OAM, Zona
---------------------------------------------------------------------------------*/
#include <stdio.h>                       /* printf.                                     */
#include <string.h>                      /* memset.                                     */
#include "juego/mundo.h"                 /* Cuerpo, mundo_actualizar, mundo_altura...   */
#include "juego/profundidad.h"           /* profundidad_ordenar.                        */
#include "juego/camara.h"                /* camara_avanzar.                             */
#include "juego/oleadas.h"               /* Oleadas, oleadas_limite...                  */
#include "juego/nivel.h"                 /* ALTO_CELDA, ANCHO_PANTALLA.                 */

#define SOMBRA_ALTO 10                   /* Copia de protagonista_sprite.c.             */
#define ENTIDADES 8                      /* Tarou, su sombra, 3 ninjas, 3 sombras.      */

/* Nombres de las 8 entradas de la tabla de entidades (el orden de ninja_sprite.c). */
static const char *const NOMBRE[ENTIDADES] = {"Tarou", "sTarou", "Nin1", "Nin2",
                                              "Nin3",  "sNin1",  "sNin2", "sNin3"};

/* Nombres de EstadoCuerpo (mundo.h), en el orden del enum. */
static const char *const ESTADO_NOMBRE[CUERPO_ESTADOS] = {"PARADO", "ANDANDO", "SUBIENDO", "BAJANDO",
                                                          "ATACANDO", "RECIBIR", "DERRIBADO"};

/*--- Parte 1: el salto en z ---------------------------------------------------------*/
static void parte1_salto(void)
{
    Cuerpo c;
    Mando m;
    u16 frame, apogeo = 0, altura_max = 0;
    s16 z_max = 0;

    printf("=== Parte 1: el salto en z (mundo.c real: SALTO_V0 %d, GRAVEDAD %d) ===\n",
           SALTO_V0, GRAVEDAD);
    printf("Los pies estan en y = 200 todo el rato; la y de pantalla del cuerpo es y - altura.\n");
    printf("%5s %6s %6s %6s %4s %12s %12s  %s\n", "frame", "z", "altura", "vz", "y",
           "cuerpo y0", "sombra y0", "estado");
    memset(&m, 0, sizeof(m));
    mundo_iniciar(&c, 120, 200);
    m.saltar_pulsado = 1;                            /* Un B recien pulsado en el frame 1. */
    for (frame = 1; frame <= 60; frame++)
    {
        mundo_actualizar(&c, &m);
        m.saltar_pulsado = 0;                        /* Solo ese frame.                    */
        if (c.z > z_max)
        {
            z_max = c.z;
            apogeo = frame;
            altura_max = mundo_altura(&c);
        }
        printf("%5u %6d %6u %6d %4u %12d %12d  %s\n", frame, c.z, mundo_altura(&c), c.vz,
               c.y, c.y - ALTO_CELDA + 2 - mundo_altura(&c), c.y - SOMBRA_ALTO, ESTADO_NOMBRE[c.estado]);
        if (c.z == 0)
        {
            break;                                   /* Aterrizo.                          */
        }
    }
    printf("Apogeo en el frame %u (z = %d = %u px), aterriza en el frame %u.\n\n", apogeo,
           z_max, altura_max, frame);
}

/*--- Contar el coste de la ordenacion por insercion (copia instrumentada) -----------*/
static void ordenar_contando(u8 *orden, const u16 *y, u8 n, u16 *comparaciones, u16 *movidas)
{
    u8 i, j, actual;
    u16 y_actual;

    *comparaciones = 0;
    *movidas = 0;
    for (i = 1; i < n; i++)
    {
        actual = orden[i];
        y_actual = y[actual];
        j = i;
        while (j > 0)
        {
            (*comparaciones)++;                      /* Se mira la y de la de delante.     */
            if (!(y[orden[j - 1]] < y_actual))
            {
                break;                               /* Ya esta en su sitio.               */
            }
            orden[j] = orden[j - 1];                 /* Se corre un sitio.                 */
            (*movidas)++;
            j--;
        }
        orden[j] = actual;
    }
}

/*--- Parte 2: el orden por profundidad, cuadro a cuadro -----------------------------*/
static void parte2_orden(void)
{
    u16 ent_y[ENTIDADES];
    u8 orden[ENTIDADES], antes[ENTIDADES], copia[ENTIDADES];
    u16 comparaciones, movidas, comp_total = 0, mov_total = 0;
    u8 k, cuadro, cambio, tarou_y;

    printf("=== Parte 2: orden de dibujo por la y de los pies (profundidad_ordenar real) ===\n");
    printf("Las sombras llevan y = 0: acaban las ultimas de la OAM, detras de todo.\n");
    printf("Ninjas con y = 190, 204 y 204 (los dos ultimos empatan: el orden estable los respeta).\n\n");
    ent_y[0] = 184;                                  /* Tarou arranca en lo alto del carril.*/
    ent_y[1] = 0;                                    /* Su sombra.                          */
    ent_y[2] = 190;                                  /* Ninja 1.                            */
    ent_y[3] = 204;                                  /* Ninja 2.                            */
    ent_y[4] = 204;                                  /* Ninja 3: empata con el 2.           */
    ent_y[5] = ent_y[6] = ent_y[7] = 0;              /* Sus sombras.                        */
    for (k = 0; k < ENTIDADES; k++)
    {
        orden[k] = k;                                /* sprites_usar(): orden de partida.   */
    }
    printf("%5s %5s  %-52s %s\n", "frame", "yTar", "orden de la OAM (delante -> detras)",
           "comp. mov.");
    for (cuadro = 0; cuadro <= 34; cuadro++)
    {
        tarou_y = (u8)(184 + cuadro);                /* Baja 1 px por frame (PASO_Y).       */
        ent_y[0] = tarou_y;
        memcpy(antes, orden, sizeof(orden));
        memcpy(copia, orden, sizeof(orden));
        ordenar_contando(copia, ent_y, ENTIDADES, &comparaciones, &movidas);
        profundidad_ordenar(orden, ent_y, ENTIDADES);    /* La del juego.                   */
        if (memcmp(copia, orden, sizeof(orden)) != 0)    /* La copia instrumentada y la real */
        {                                                /* tienen que dar lo mismo.        */
            printf("ERROR: la copia instrumentada no coincide con la real\n");
        }
        comp_total = (u16)(comp_total + comparaciones);
        mov_total = (u16)(mov_total + movidas);
        cambio = (u8)(memcmp(antes, orden, sizeof(orden)) != 0);
        if (cuadro % 4 == 0 || cambio)               /* Solo los frames que cuentan.        */
        {
            printf("%5u %5u  ", cuadro, tarou_y);
            for (k = 0; k < ENTIDADES; k++)
            {
                printf("%s%s", NOMBRE[orden[k]], k + 1 < ENTIDADES ? " " : "");
            }
            printf("  %4u %4u%s\n", comparaciones, movidas, cambio ? "   <- cambia el orden" : "");
        }
    }
    printf("En 35 cuadros: %u comparaciones y %u movidas en total (n = 8: el minimo posible\n"
           "es n - 1 = 7 comparaciones por cuadro).\n\n", comp_total, mov_total);
}

/*--- Parte 3: quien tapa a quien -----------------------------------------------------*/
/* Dibuja Tarou (T, cuerpo de 8x5 caracteres), su sombra (s, 12x1) y un ninja (N, 8x5): en cada
   caracter gana la PRIMERA entidad de `orden` (la entrada mas baja de la OAM) que lo cubre.
   La fila de los pies es (y - 176) / 4: una fila de la figura son 4 px del carril. */
static void dibujar_oam(const u8 *orden, const u16 *ent_y, const char *titulo)
{
    char figura[11][17];
    int fila, col, k, quien, pies, x0, ancho, alto, cubre;

    printf("%s\n", titulo);
    for (fila = 0; fila < 11; fila++)
    {
        for (col = 0; col < 16; col++)
        {
            figura[fila][col] = '.';
            for (k = 0; k < 3; k++)                  /* Por orden de OAM: entidades 0, 1, 2. */
            {
                quien = orden[k];                    /* (la tabla de la demo tiene solo 3)  */
                pies = ((int)ent_y[quien == 1 ? 0 : quien] - 176) / 4;   /* Sombra: la y de Tarou. */
                if (quien == 1)
                {
                    x0 = 4; ancho = 12; alto = 1;    /* La sombra: ancha y de una fila.     */
                }
                else
                {
                    x0 = (quien == 0) ? 6 : 2; ancho = 8; alto = 5;
                }
                cubre = (col >= x0 && col < x0 + ancho && fila > pies - alto && fila <= pies);
                if (cubre)
                {
                    figura[fila][col] = (quien == 0) ? 'T' : (quien == 1 ? 's' : 'N');
                    break;                           /* La primera que cubre gana.          */
                }
            }
        }
        figura[fila][16] = '\0';
        printf("    %s\n", figura[fila]);
    }
}

static void parte3_tapa(void)
{
    u16 ent_y[3];
    u8 orden[3];
    u8 k;

    printf("=== Parte 3: gana la entrada MAS BAJA de la OAM (T Tarou, N ninja, s sombra) ===\n");
    memset(ent_y, 0, sizeof(ent_y));
    ent_y[0] = 210;                                  /* Tarou, MAS ABAJO (cerca de la camara). */
    ent_y[2] = 190;                                  /* El ninja, mas arriba (lejos).       */
    for (k = 0; k < 3; k++)
    {
        orden[k] = k;                                /* Tabla de 3: 0 Tarou, 1 su sombra, 2 el ninja. */
    }
    dibujar_oam(orden, ent_y, "a) Tarou delante (y 210 > 190), tabla SIN ordenar [0 = Tarou ya va primero]:");
    ent_y[0] = 180;                                  /* Ahora Tarou pasa por DETRAS del ninja. */
    dibujar_oam(orden, ent_y,
                "b) Tarou detras (y 180 < 190), tabla SIN ordenar: error, Tarou sigue tapando al ninja:");
    profundidad_ordenar(orden, ent_y, 3);
    dibujar_oam(orden, ent_y, "c) El mismo caso b) ORDENADO: el ninja (y 190) va primero y tapa a Tarou:");
    printf("\n");
}

/*--- Parte 4: la camara con tope de zona ------------------------------------------*/
static void parte4_camara(void)
{
    Cuerpo c;
    Mando m;
    Oleadas o;
    u16 camara = 0, frame;
    u8 vencida = 0;

    printf("=== Parte 4: camara_avanzar + oleadas (orden de pantalla_fase1.c) ===\n");
    printf("Tarou camina siempre a la derecha. En el frame 260 se 'vence la oleada' (A, hoy).\n");
    printf("%5s %6s %6s %5s %5s  %s\n", "frame", "Tar x", "camara", "zona", "bloq", "limite de la camara");
    memset(&m, 0, sizeof(m));
    mundo_iniciar(&c, 64, 200);                      /* Como X_INICIAL e Y_INICIAL.         */
    oleadas_iniciar(&o);
    m.derecha = 1;
    for (frame = 1; frame <= 600; frame++)
    {
        mundo_actualizar(&c, &m);                    /* 1. el cuerpo...                     */
        mundo_limitar_x(&c, (u16)(camara + MARGEN_PANTALLA),          /* 2. ...sin salir de la */
                        (u16)(camara + ANCHO_PANTALLA - MARGEN_PANTALLA));   /* pantalla.  */
        camara = camara_avanzar(camara, c.x, oleadas_limite(&o));    /* 3. la camara.       */
        oleadas_actualizar(&o, camara);              /* 4. ¿llego a la zona? bloquea.       */
        if (frame == 260 && o.bloqueada)
        {
            oleadas_vencer(&o);                      /* La oleada se vence: libre.          */
            vencida = 1;
        }
        if (frame == 1 || frame % 40 == 0 || frame == 261 || camara == 640)
        {
            printf("%5u %6u %6u %5u %5u  0x%04X%s\n", frame, c.x, camara, o.actual,
                   o.bloqueada, oleadas_limite(&o),
                   (frame == 261 && vencida) ? "   <- oleada vencida, la camara se libera" : "");
        }
        if (camara == 640)
        {
            break;                                   /* Ya llego al punto de la zona 1.     */
        }
    }
    printf("Frame %u: la camara llega a %u (zona %u bloqueada otra vez) y Tarou va en x = %u (su tope derecho: camara + 256 - 24 = %u).\n",
           frame, camara, o.actual, c.x, camara + ANCHO_PANTALLA - MARGEN_PANTALLA);
}

int main(void)
{
    parte1_salto();
    parte2_orden();
    parte3_tapa();
    parte4_camara();
    return 0;
}
