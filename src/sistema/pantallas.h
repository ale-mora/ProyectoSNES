/*---------------------------------------------------------------------------------
    pantallas.h — Lista de pantallas de la secuencia, sus tiempos en frames y las
    funciones que cada pantalla ofrece al resto del programa.

    PROPÓSITO
      Es un header: no contiene código que se ejecute, solo declaraciones. Aquí
      está el enum Pantalla (el orden ES el orden de la secuencia), las
      duraciones (SEGUNDOS(n) → frames) y los prototipos pantalla_xxx_iniciar/
      _actualizar(/_vblank) que secuencia.c llama.

    SE CONECTA CON
      Incluye:  <snes.h> (tipos u8/u16 y la API de la SNES).
      Lo usan:  sistema/secuencia.h/.c y cada pantalla_xxx.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (globales y estáticas de este archivo)
      ninguna; constantes FPS, SEGUNDOS(), DURACION_*, ATTRACT_SIN_INPUT,
      FRAMES_PARPADEO, FRAMES_FADE, FRAMES_FADE_LENTO, FRAMES_POR_LETRA,
      PERMITE_SALTO. OJO: tras cambiar este archivo hay que hacer `make clean`
      (snes_rules no rastrea cabeceras).

    → glosario: #include, #define, #ifndef, enum, Frame, void
---------------------------------------------------------------------------------*/
#ifndef PANTALLAS_H   /* Guarda de inclusión: si PANTALLAS_H aún no está definido... */
#define PANTALLAS_H   /* ...lo definimos y seguimos; la 2.ª vez se salta todo. → glosario: #ifndef */

#include <snes.h>     /* La librería PVSnesLib: tipos u8/u16 y todas las funciones de la SNES. */

/*--- Identificador de cada pantalla ---------------------------------------------
    Un enum es una lista de nombres a los que C asigna 0, 1, 2... en orden.
    El orden aquí ES el orden de la secuencia (convención de época).
    CANTIDAD_PANTALLAS queda al final y vale 8: cuántas pantallas hay.
    → glosario: enum, Máquina de estados */
typedef enum
{
    PANTALLA_NINTENDO = 0,     /* (1) logo Nintendo + "LICENSED BY NINTENDO", muda        */
    PANTALLA_PUBLISHER,        /* (2) logo del publisher (provisional: Capcom del SDK)      */
    PANTALLA_DEVS,             /* (3) logo del estudio en Mode 7 + nombre y roles            */
    PANTALLA_DEDICATORIA,      /* (4) mensaje personal, máquina de escribir                  */
    PANTALLA_INTRO,            /* (5) tres escenas con texto                                 */
    PANTALLA_TITULO,           /* (6) título con PRESS START, attract a los 15 s             */
    PANTALLA_NIVEL1,           /* (7) el nivel 1 (el tutorial); Start vuelve al título       */
    PANTALLA_PROXIMAMENTE,     /* (8) "LEVEL 2 / COMING SOON" al salir del nivel 1           */
    CANTIDAD_PANTALLAS         /* no es una pantalla: cuenta cuántas hay arriba              */
} Pantalla;                    /* "Pantalla" pasa a ser un tipo de dato, como u8 o u16.      */

/*--- Tiempos (en frames) --------------------------------------------------------
    La SNES NTSC dibuja 60 imágenes por segundo, así que 1 segundo = 60 frames.
    Escribimos SEGUNDOS(3) en vez de 180 para que se lea la intención.
    El (u16) convierte el resultado al tipo entero de 16 bits que usamos para
    contar frames (hasta 65535 frames = 18 minutos). → glosario: Frame, u8/u16 */
#define FPS 60
#define SEGUNDOS(s) ((u16)((s) * FPS))

#define DURACION_NINTENDO     SEGUNDOS(3)   /* logo Nintendo                            */
#define DURACION_PUBLISHER    SEGUNDOS(3)   /* logo publisher                           */
#define DURACION_DEVS         SEGUNDOS(4)   /* 2 s de zoom Mode 7 + 2 s quieto          */
#define DURACION_DEDICATORIA  SEGUNDOS(7)   /* mensaje personal                         */
#define DURACION_ESCENA_INTRO SEGUNDOS(8)   /* cada una de las 3 escenas                */
#define ATTRACT_SIN_INPUT     SEGUNDOS(15)  /* título → intro si nadie toca nada        */
#define DURACION_PROXIMAMENTE SEGUNDOS(4)   /* "COMING SOON" tras el nivel 1            */
#define FRAMES_PARPADEO       30            /* PRESS START: 0,5 s visible, 0,5 s oculto */
/* OJO: setFadeEffectEx recibe frames POR CADA PASO de brillo, y hay 16 pasos
   (0..15). Duración real = valor * 16 frames: 2 → 32 frames (~0,5 s),
   4 → 64 frames (~1 s). Verificado en videos.asm del SDK el 2026-09-24. */
#define FRAMES_FADE           2             /* fundidos normales: 2*16 = 32 frames      */
#define FRAMES_FADE_LENTO     4             /* dedicatoria: 4*16 = 64 frames            */
#define FRAMES_POR_LETRA      3             /* máquina de escribir: 1 letra cada 3 frames */

/*--- Salto de pantallas ---------------------------------------------------------
    Decisión de diseño: por ahora NINGUNA pantalla se salta. Se deja el
    interruptor listo: poner 1 aquí y las pantallas que lo consulten permitirán
    saltar con Start. */
#define PERMITE_SALTO 0

/*--- Funciones que cada pantalla ofrece -----------------------------------------
    Cada pantalla vive en su propio archivo pantalla_xxx.c y expone dos
    funciones con el mismo patrón:
      - xxx_iniciar():    se llama UNA vez al entrar (dibuja, carga, fade in).
      - xxx_actualizar(): se llama UNA vez POR FRAME mientras la pantalla está
                          activa (cuenta tiempo, lee el mando, decide cuándo
                          pasar a la siguiente).
    "void" = no devuelve nada; "(void)" = no recibe parámetros. → glosario: void */
void pantalla_nintendo_iniciar(void);
void pantalla_nintendo_actualizar(void);

void pantalla_publisher_iniciar(void);
void pantalla_publisher_actualizar(void);

void pantalla_devs_iniciar(void);
void pantalla_devs_actualizar(void);

void pantalla_dedicatoria_iniciar(void);
void pantalla_dedicatoria_actualizar(void);

void pantalla_intro_iniciar(void);
void pantalla_intro_actualizar(void);

void pantalla_titulo_iniciar(void);
void pantalla_titulo_actualizar(void);

/* El nivel 1 es la única pantalla con _vblank: copia a VRAM los tiles del sprite
   dinámico justo después del VBlank (ver secuencia_vblank en secuencia.c). */
void pantalla_nivel1_iniciar(void);
void pantalla_nivel1_actualizar(void);
void pantalla_nivel1_vblank(void);

void pantalla_proximamente_iniciar(void);
void pantalla_proximamente_actualizar(void);

#endif /* PANTALLAS_H — cierra la guarda de inclusión abierta arriba. */
