/*---------------------------------------------------------------------------------
    ninja.h — El ninja kendoka de prueba: se acerca, ataca con el shinai y recibe golpes.

    PROPÓSITO
      El enemigo básico del beat 'em up (docs/diseno-beat-em-up.md §5.5 y §6),
      en su versión mínima: vida y daño por rango de cinturón (cifras.h), un
      ataque de shinai con su caja de golpe, un enfriamiento entre ataques y
      las mismas reacciones que Tarou (recibir, derribo, KO). No decide solo
      cuándo puede atacar: el llamador le da `permiso` (los turnos de ataque de
      §6: como mucho 2 a la vez) y él solo ataca si además está en rango y no
      está en enfriamiento.
      Lógica pura, probada en el PC. Usa el mundo x/y/z (mundo.h), la regla de
      golpe (combate.h) y el Golpeable de siempre.

    SE CONECTA CON
      Incluye:  sistema/tipos.h, juego/mundo.h, juego/combate.h, juego/cifras.h.
      Lo usan:  tests/prueba_ninja.c y, cuando exista, la pantalla del prototipo.

    HARDWARE QUE TOCA
      ninguno (lógica pura).

    VARIABLES (campos de Ninja: 32 bytes, potencia de 2)
      c               Cuerpo     posición, velocidad y altura
      g               Golpeable  vida (por rango), invulnerable, destello, parada
      estado          u8         un valor de NinjaEstado
      tiempo          u8         frames en el estado actual (satura en 255)
      rango           u8         0 blanco .. 3 rojo (cinturón)
      espera          u8         frames hasta poder atacar otra vez
      mira_izquierda  u8         1 si mira a la izquierda
      vencido         u8         1 cuando el KO ha terminado
      relleno        u8[6]      hasta 32 bytes

    → glosario: Rango, Turnos de ataque, Caja de golpe / caja de daño, Derribo
---------------------------------------------------------------------------------*/
#ifndef NINJA_H
#define NINJA_H

#include "sistema/tipos.h"
#include "juego/mundo.h"
#include "juego/combate.h"

/* Celda del ninja (64x96): los pies están en el centro de la última fila. */
#define NINJA_ANCHO 64
#define NINJA_MITAD 32
#define NINJA_ALTO 96

/* Frames entre dos ataques del mismo ninja. */
#define COOLDOWN_NINJA 60

/* Estados (prefijo NQ_ para no chocar con LQ_ de luchador.h). */
enum NinjaEstado
{
    NQ_QUIETO = 0,                   /* Parado: espera permiso o está en rango sin atacar. */
    NQ_ACERCARSE,                    /* Andando hacia el objetivo.                         */
    NQ_ATACAR,                       /* Shinai: caja de golpe en sus frames activos.       */
    NQ_RECIBIR,                      /* Retrocede por un golpe normal.                     */
    NQ_DERRIBADO,                    /* Volando por un derribo, hasta aterrizar.           */
    NQ_EN_SUELO,                     /* Tirado en el suelo.                                */
    NQ_LEVANTARSE,                   /* Se levanta: invulnerable mientras tanto.          */
    NQ_KO,                           /* Vencido: al acabar su tiempo, sale de la pelea.    */
    NQ_ESTADOS
};

typedef struct
{
    Cuerpo c;                        /* Posición, velocidad y altura.                      */
    Golpeable g;                     /* Vida y contadores.                                 */
    u8 estado;                       /* Un valor de NinjaEstado.                           */
    u8 tiempo;                       /* Frames en el estado actual.                        */
    u8 rango;                        /* 0 blanco .. 3 rojo.                                */
    u8 espera;                       /* Frames hasta poder atacar otra vez.                */
    u8 mira_izquierda;               /* 1 si mira a la izquierda.                          */
    u8 vencido;                      /* 1 cuando el KO ha terminado.                       */
    u8 relleno[6];                   /* Hasta 32 bytes.                                    */
} Ninja;

/* Lo deja de pie en (x, y) con la vida y el daño de su rango. `rango` 0..3. */
void ninja_iniciar(Ninja *n, u16 x, u16 y, u8 rango);

/* Un frame. `objetivo` es el cuerpo del protagonista; `permiso` es 1 si el llamador
   deja atacar a este ninja (turnos de ataque). */
void ninja_actualizar(Ninja *n, const Cuerpo *objetivo, u8 permiso, const Carril *carril);

/* Reacciona al resultado de combate_golpear sobre este ninja (GOLPE_*). */
void ninja_reaccionar(Ninja *n, u8 resultado);

/* Su caja de golpe de este frame (shinai), o 0 si no pega. Rellena `salida`. */
u8 ninja_caja_golpe(const Ninja *n, CajaCuadro *salida);

/* Su caja de daño: dónde le pueden golpear (el cuerpo entero). Rellena `salida`. */
void ninja_caja_dano(const Ninja *n, CajaCuadro *salida);

#endif /* NINJA_H */
