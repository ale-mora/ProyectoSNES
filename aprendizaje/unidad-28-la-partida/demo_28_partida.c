/*---------------------------------------------------------------------------------
    demo_28_partida.c — La partida alrededor del combate, con el código REAL:
    los personajes como datos (juego/personaje.c) y la máquina de la partida
    (juego/estado_fase.c).

    PROPÓSITO
      A. Los dos personajes, lado a lado: misma lista de animaciones (AnimProtaId), tablas
         distintas (qué tira, cuántos cuadros, cada cuántos frames), celda y alcance. El
         resto del juego no pregunta «¿es Tarou?»: pide la tabla del personaje.
      B. Una partida inventada, frame a frame, por estado_fase_actualizar: jugar, pausar,
         Select en pausa, KO, la cuenta del CONTINUE, Start para continuar y la fase
         vencida. Por cada cambio se imprime el estado nuevo, la acción que devuelve y si
         el mundo avanza y la pantalla está a media luz.
      C. La cuenta atrás entera sin pulsar nada: cuánto tarda el GAME OVER en volver al
         título.

    SE CONECTA CON
      Usa:  src/juego/personaje.c (personaje_anims, personaje_ancho/_alto/_alcance),
            src/juego/estado_fase.c, res/personajes/tarou/tarou_anim.h y barbara/barbara_anim.h.
      Imita: src/pantallas/pantalla_fase1.c (pantalla_fase1_actualizar: le pasa los botones
             recién pulsados y si hay KO o fase vencida, y obedece a la acción).

    HARDWARE QUE TOCA
      ninguno (PC).

    VARIABLES: ninguna global.
    Compilar (desde esta carpeta):
      gcc -std=c99 -Wall -Wextra -DPRUEBAS_PC -I../../src -I../../res/personajes/tarou -I../../res/personajes/barbara -o demo_28.exe demo_28_partida.c ../../src/juego/personaje.c ../../src/juego/estado_fase.c ../../src/juego/mundo.c ../../src/juego/animaciones.c
    → glosario: Máquina de estados, Tabla de animaciones
---------------------------------------------------------------------------------*/
#include <stdio.h>
#include "juego/personaje.h"
#include "juego/estado_fase.h"

static const char *NOMBRE_ANIM[ANIM_PROTA_CANTIDAD] = {
    "guardia", "caminar", "saltar", "caer", "golpe 1", "golpe 2", "golpe 3", "recibir", "ko"};
static const char *NOMBRE_ESTADO[] = {"JUGANDO", "PAUSA", "KO", "GAME OVER", "STAGE CLEAR"};
static const char *NOMBRE_ACCION[] = {"-", "TITULO", "REINICIAR", "PROXIMAMENTE"};

/* Un suceso del guion: en el frame `f`, estos botones (recién pulsados) y estas condiciones. */
typedef struct
{
    unsigned f;
    u8 start, select, ko, vencida;
    const char *que;
} Suceso;

static void personajes(void)
{
    const AnimProta *t = personaje_anims(PERSONAJE_TAROU);
    const AnimProta *b = personaje_anims(PERSONAJE_BARBARA);
    u8 k;

    printf("A. Los personajes como datos\n");
    printf("   %-8s | %-22s | %-22s\n", "", "Tarou", "Barbara");
    printf("   %-8s | celda %3ux%-3u alc. %3u | celda %3ux%-3u alc. %3u\n", "",
           personaje_ancho(PERSONAJE_TAROU), personaje_alto(PERSONAJE_TAROU),
           personaje_alcance(PERSONAJE_TAROU), personaje_ancho(PERSONAJE_BARBARA),
           personaje_alto(PERSONAJE_BARBARA), personaje_alcance(PERSONAJE_BARBARA));
    for (k = 0; k < ANIM_PROTA_CANTIDAD; k++)
    {
        printf("   %-8s | tira %2u %2u cuadros x%u %s | tira %2u %2u cuadros x%u %s\n",
               NOMBRE_ANIM[k], t[k].tira, t[k].cuadros, t[k].frames, t[k].bucle ? "bucle" : "     ",
               b[k].tira, b[k].cuadros, b[k].frames, b[k].bucle ? "bucle" : "     ");
    }
}

static void partida(void)
{
    static const Suceso guion[] = {
        {100, 1, 0, 0, 0, "Start: pausa"},
        {160, 1, 0, 0, 0, "Start: sigue"},
        {300, 1, 0, 0, 0, "Start: pausa otra vez"},
        {310, 0, 1, 0, 0, "Select en pausa"},
    };
    static const Suceso guion2[] = {
        {50, 0, 0, 1, 0, "la protagonista cae KO (y sigue KO)"},
        {300, 1, 0, 1, 0, "Start durante la cuenta"},
    };
    EstadoFase e;
    unsigned f, k;
    u8 antes, a, ko;

    printf("\nB. Una partida, frame a frame (solo los cambios)\n");
    estado_fase_iniciar(&e);
    antes = 0xFF;
    k = 0;
    for (f = 0; f < 400; f++)
    {
        u8 start = 0, select = 0;
        if (k < sizeof guion / sizeof guion[0] && guion[k].f == f)
        {
            start = guion[k].start;
            select = guion[k].select;
            printf("   [f=%3u] %s\n", f, guion[k].que);
            k++;
        }
        a = estado_fase_actualizar(&e, start, select, 0, 0);
        if (e.estado != antes || a != ACCION_NADA)
        {
            printf("   [f=%3u] -> %-11s accion %-12s mundo %s  media luz %s\n", f,
                   NOMBRE_ESTADO[e.estado], NOMBRE_ACCION[a],
                   estado_fase_mundo_avanza(&e) ? "avanza " : "quieto ",
                   estado_fase_atenuada(&e) ? "si" : "no");
            antes = e.estado;
        }
        if (a != ACCION_NADA)
        {
            printf("   (la pantalla sale: salir(PANTALLA_TITULO))\n");
            break;
        }
    }

    printf("\n   Otra partida: KO, GAME OVER y CONTINUE\n");
    estado_fase_iniciar(&e);
    antes = 0xFF;
    k = 0;
    ko = 0;
    for (f = 0; f < 400; f++)
    {
        u8 start = 0;
        if (k < sizeof guion2 / sizeof guion2[0] && guion2[k].f == f)
        {
            start = guion2[k].start;
            ko = 1;
            printf("   [f=%3u] %s\n", f, guion2[k].que);
            k++;
        }
        a = estado_fase_actualizar(&e, start, 0, ko, 0);
        if (e.estado != antes || a != ACCION_NADA)
        {
            printf("   [f=%3u] -> %-11s accion %-12s CONTINUE? %u\n", f, NOMBRE_ESTADO[e.estado],
                   NOMBRE_ACCION[a], e.cuenta);
            antes = e.estado;
        }
        if (a != ACCION_NADA)
        {
            printf("   (la pantalla sale: salir(PANTALLA_FASE1) = la fase desde el principio)\n");
            break;
        }
    }

    printf("\n   Y otra: la ultima oleada cae en el frame 20\n");
    estado_fase_iniciar(&e);
    for (f = 0; f < 400; f++)
    {
        a = estado_fase_actualizar(&e, (u8)(f == 100), 0, 0, (u8)(f >= 20));
        if (f == 20 || a != ACCION_NADA)
        {
            printf("   [f=%3u] -> %-11s accion %s%s\n", f, NOMBRE_ESTADO[e.estado], NOMBRE_ACCION[a],
                   f == 20 ? "   (Start en el frame 100: no corta el STAGE CLEAR)" : "");
        }
        if (a != ACCION_NADA)
        {
            break;
        }
    }
}

static void cuenta(void)
{
    EstadoFase e;
    unsigned f = 0;
    u8 a = ACCION_NADA, ultima = 0xFF;

    printf("\nC. El GAME OVER sin tocar nada\n   ");
    estado_fase_iniciar(&e);
    while (a == ACCION_NADA)
    {
        a = estado_fase_actualizar(&e, 0, 0, 1, 0);
        if (e.estado == FASE_GAME_OVER && e.cuenta != ultima)
        {
            printf("%u@%u ", e.cuenta, f);
            ultima = e.cuenta;
        }
        f++;
    }
    printf("\n   accion %s en el frame %u (%u s a 60 Hz: 1 de KO + 10 de cuenta)\n",
           NOMBRE_ACCION[a], f - 1, f / 60);
}

int main(void)
{
    personajes();
    partida();
    cuenta();
    return 0;
}
