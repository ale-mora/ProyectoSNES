/*---------------------------------------------------------------------------------
    demo_08_enum_switch_const.c — Demo 8 de la unidad 00: enum, switch y tablas const.

    PROPÓSITO
      Enseña, en cuatro pasos numerados en la salida: 1) un enum con nombre
      (typedef enum, como Pantalla en src/sistema/pantallas.h y Estado en
      src/juego/animaciones.h) y su último elemento como «cuántos hay»;
      2) tablas const indexadas con ese enum (como ANIMACIONES[] y
      NOMBRES_ESTADO[] en src/juego/animaciones.c); 3) switch / case / break /
      default, con casos agrupados y con return dentro de un case (como
      secuencia.c, tarou.c:509 y tarou_sprite.c:105); 4) una tabla const de
      eventos escrita con macros y recorrida con un puntero hasta su marca de
      fin, como ESCENA_NINTENDO en src/escena/escena_nintendo.c.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf), <stdint.h> (uint8_t, uint16_t).
      Lo usan:  nadie: es un programa independiente de PC.
      Modelo:   src/sistema/pantallas.h, src/sistema/secuencia.c:52-66,
                src/juego/animaciones.h/.c, src/escena/escena_nintendo.c.
      Compilar: gcc -std=c99 -Wall -Wextra -o demo_08.exe demo_08_enum_switch_const.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      DURACION[4]   const u16           frames de cada pantalla (0 = espera al jugador)
      NOMBRE[4]     const char *const   nombre de cada pantalla
      ESCENA[]      const Evento        guion de eventos, terminado en PISTA_FIN
      NOMBRE_PISTA  const char *const   "brillo " o "mosaico", por número de pista
      p             u8                  pantalla que se recorre (0..4)
      e             const Evento *      evento que se lee

    → conceptos (README de la unidad 00): enum, switch, case, break, default, const
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */
#include <stdint.h>                  /* Trae los enteros de tamaño fijo.                  */

typedef uint8_t u8;                  /* Los nombres del juego (src/sistema/tipos.h).      */
typedef uint16_t u16;

#define FPS 60                       /* Frames por segundo.                               */
#define SEGUNDOS(s) ((u16)((s) * FPS))   /* Igual que en pantallas.h.                     */

/*--- 1. Un enum: nombres para 0, 1, 2... El último cuenta cuántos hay ---------------*/
typedef enum
{
    PANTALLA_NINTENDO = 0,           /* 0                                                 */
    PANTALLA_PUBLISHER,              /* 1: sin "=", vale el anterior + 1.                 */
    PANTALLA_TITULO,                 /* 2                                                 */
    PANTALLA_NIVEL1,                 /* 3                                                 */
    CANTIDAD_PANTALLAS               /* 4: no es una pantalla, es el tamaño de las tablas. */
} Pantalla;

/*--- 2. Tablas const: una fila por pantalla, en el MISMO orden que el enum ----------*/
static const u16 DURACION[CANTIDAD_PANTALLAS] = {
    SEGUNDOS(3),                     /* PANTALLA_NINTENDO                                 */
    SEGUNDOS(3),                     /* PANTALLA_PUBLISHER                                */
    0,                               /* PANTALLA_TITULO: espera a START.                  */
    0,                               /* PANTALLA_NIVEL1: hasta que se termina.            */
};
static const char *const NOMBRE[CANTIDAD_PANTALLAS] = {   /* Dos const: ver README.    */
    "NINTENDO", "PUBLISHER", "TITULO", "NIVEL1",
};

/*--- 3. switch: saltar al caso que coincide con el valor ----------------------------*/
static void iniciar(u8 p)            /* u8, como Tarou.estado: el enum solo pone nombres. */
{
    switch (p)
    {
        case PANTALLA_NINTENDO:      /* Si p vale 0...                                    */
            printf("3. iniciar: logo con mosaico y destello\n");
            break;                   /* ...hace esto y sale del switch.                   */
        case PANTALLA_PUBLISHER:
            printf("3. iniciar: logo del publisher\n");
            break;
        case PANTALLA_TITULO:        /* Dos casos agrupados: comparten el código          */
        case PANTALLA_NIVEL1:        /* (como los estados de suelo en tarou.c:509).       */
            printf("3. iniciar: pantalla que usa el mando\n");
            break;
        default:                     /* Cualquier otro valor.                             */
            printf("3. iniciar: pantalla desconocida (%d)\n", p);
            break;
    }
}

/* return dentro de un case: sale de la función entera, no hace falta break
   (así es tarou_sprite_tabla en src/juego/tarou_sprite.c:105). */
static const char *musica(u8 p)
{
    switch (p)
    {
        case PANTALLA_TITULO: return "tema del titulo";
        case PANTALLA_NIVEL1: return "tema del bosque";
        default:              return "silencio";
    }
}

/*--- 4. Una tabla de eventos escrita con macros, como escena_nintendo.c --------------*/
#define PISTA_BRILLO 0               /* Qué valor cambia un evento.                       */
#define PISTA_MOSAICO 1
#define PISTA_FIN 255                /* Marca de fin de la tabla.                         */

/* Nombre de cada pista, por su número (en vez de un ?: con cadenas, que tcc816 no
   compila como argumento de una función variádica: ver CLAUDE.md). */
static const char *const NOMBRE_PISTA[2] = {"brillo ", "mosaico"};

typedef struct
{
    u16 frame;                       /* En qué frame ocurre.                              */
    u8 pista;                        /* PISTA_*.                                          */
    u8 valor;                        /* Valor nuevo.                                      */
} Evento;                            /* 4 bytes: potencia de 2 (ver CLAUDE.md).           */

#define FIJAR(f, pista, v) {(f), (pista), (v)}   /* Una fila de la tabla.                 */
#define FIN_ESCENA(f) {(f), PISTA_FIN, 0}        /* La última fila.                       */

static const Evento ESCENA[] = {     /* Sin tamaño: lo cuenta el compilador.              */
    FIJAR(0, PISTA_MOSAICO, 15),
    FIJAR(0, PISTA_BRILLO, 0),
    FIJAR(32, PISTA_BRILLO, 15),
    FIJAR(48, PISTA_MOSAICO, 0),
    FIN_ESCENA(244),
};

int main(void)
{
    /* 1. Los valores del enum. */
    printf("1. PANTALLA_NINTENDO=%d PANTALLA_NIVEL1=%d CANTIDAD_PANTALLAS=%d\n",
           PANTALLA_NINTENDO, PANTALLA_NIVEL1, CANTIDAD_PANTALLAS);

    /* 2. Recorrer las tablas con el enum como índice. */
    for (u8 p = 0; p < CANTIDAD_PANTALLAS; p++)
    {
        printf("2. %d %-9s dura %3d frames, musica: %s\n",   /* %-9s: texto alineado a 9. */
               p, NOMBRE[p], DURACION[p], musica(p));
    }

    /* 3. El switch, con cada pantalla y con un valor fuera de la lista. */
    for (u8 p = 0; p <= CANTIDAD_PANTALLAS; p++)   /* <=: también el 4, que no es pantalla. */
    {
        iniciar(p);
    }

    /* 4. Recorrer la tabla de eventos con un puntero hasta la marca de fin. */
    const Evento *e;                 /* Puntero a const: puede moverse, no escribir.      */
    for (e = ESCENA; e->pista != PISTA_FIN; e++)   /* e++ avanza una fila entera.       */
    {
        printf("4. frame %3d: %s = %d\n", e->frame, NOMBRE_PISTA[e->pista], e->valor);
    }
    printf("4. fin en el frame %d; la tabla tiene %u filas de %u bytes\n",
           e->frame, (unsigned)(sizeof(ESCENA) / sizeof(ESCENA[0])), (unsigned)sizeof(Evento));

    return 0;                        /* Terminó bien.                                     */
}
