/*---------------------------------------------------------------------------------
    luchador.h — Tarou en combate: estados, combo, derribo y agarre (spec §5.1).

    PROPÓSITO
      Es la máquina de estados del protagonista en el beat 'em up: lo que hace
      cada frame según el mando (andar, correr con doble toque, saltar, atacar,
      encadenar el combo, especial A+B, agarrar y lanzar) y lo que le hacen los
      demás (recibir, derribo, KO, ser agarrado o lanzado). Usa el mundo x/y/z
      (mundo.h), las cifras (cifras.h) y las reglas de golpe (combate.h). Es
      lógica pura, probada en el PC.
      Los tiempos de cada estado son una tabla (`fases`) y sus cajas de golpe
      otra (`cajas`), en vez de un cuadro a cuadro: cuando llegue el arte de
      Pixellab, la animación real marcará estos mismos números.
      Un ataque solo sigue el combo si CONECTÓ (`luchador_marcar_conecto`) y
      se pulsó A en la ventana de encadenado (VENTANA_COMBO frames tras el
      último cuadro activo).
      Las decisiones de estado que dependen de otro luchador (el agarre, el
      lanzamiento, el golpe que conecta) las toma el llamador y llama a las
      funciones de aquí; este módulo no busca enemigos.

    SE CONECTA CON
      Incluye:  sistema/tipos.h, juego/mundo.h, juego/combate.h,
                juego/doble_toque.h, juego/mando.h.
      Lo usan:  tests/prueba_luchador.c y, cuando exista, el prototipo de combate
                (pantalla y enemigos).

    HARDWARE QUE TOCA
      ninguno (lógica pura).

    VARIABLES (campos de Luchador: 32 bytes, potencia de 2)
      c              Cuerpo      posición, velocidad y altura (mundo.h)
      g              Golpeable   vida, invulnerable, destello, parada (combate.h)
      d              DobleToque  correr con doble toque (doble_toque.h)
      estado         u8          un valor de LuchadorEstado
      tiempo         u8          frames en el estado actual (satura en 255)
      encadenar      u8          1 si se pulsó A en la ventana de encadenado
      conecto        u8          1 si el estado actual ya le dio a alguien
      pagado         u8          1 si el especial ya pagó su coste
      golpes_agarre  u8          golpes de agarre dados (máximo 3)
      mira_izquierda u8          1 si mira a la izquierda
      vencido        u8          1 cuando un KO termina y el luchador sale de la pelea
      quien          u8          QUIEN_TAROU o QUIEN_BARBARA (decide tablas y velocidades)
      (33 bytes: no es potencia de 2, pero nunca se indexa en un array: Tarou y Bárbara
      son dos instancias separadas, a las que se llega por puntero)

    → glosario: Estado, Combo, Derribo, Parada de impacto, Doble toque, Caja de golpe
---------------------------------------------------------------------------------*/
#ifndef LUCHADOR_H
#define LUCHADOR_H

#include "sistema/tipos.h"
#include "juego/mundo.h"
#include "juego/combate.h"
#include "juego/doble_toque.h"
#include "juego/mando.h"

/* Tiempos y topes (ver luchador.c). */
#define FRAMES_KO 40                 /* Lo que tarda el KO en dar al luchador por vencido. */
#define FRAMES_RECIBIR 12            /* Retroceso por un golpe normal (no está en la spec).*/
#define MAX_GOLPES_AGARRE 3          /* Golpes de agarre seguidos (§5.3).                  */

/* A quién representa el luchador: decide sus tablas (tiempos y cajas) y sus velocidades. */
enum { QUIEN_TAROU = 0, QUIEN_BARBARA = 1 };

/* Prefijo LQ_ (luchador) para no chocar con el enum Estado de animaciones.h. */
enum LuchadorEstado
{
    LQ_QUIETO = 0,                   /* Parado: espera una orden.                          */
    LQ_ANDAR,                        /* Andando con la cruceta.                            */
    LQ_CORRER,                       /* Corriendo (doble toque).                           */
    LQ_SALTO,                        /* Subiendo o cayendo del salto.                      */
    LQ_AEREO,                        /* Ataque aéreo: derriba, hasta aterrizar.            */
    LQ_PATADA,                       /* Patada voladora (decisión 40): derriba, hasta aterrizar. */
    LQ_CARRERA_ATAQUE,               /* A mientras corre: tajo en carrera, derriba.        */
    LQ_GOLPE_1,                      /* Combo: corte 1.                                    */
    LQ_GOLPE_2,                      /* Combo: corte 2.                                    */
    LQ_GOLPE_3,                      /* Combo: corte 3.                                    */
    LQ_GOLPE_4,                      /* Combo: mordisco, derriba. Cierra el combo.         */
    LQ_ESPECIAL,                     /* A+B: giro alrededor; cuesta vida si conecta.       */
    LQ_AGARRAR,                      /* Agarra a un enemigo; A da golpes, atrás+A lanza.   */
    LQ_GOLPE_AGARRE,                 /* Zarpazo mientras agarra.                           */
    LQ_LANZAR,                       /* Lanza al agarrado (derriba al que choque).         */
    LQ_AGARRADO,                     /* Lo agarra un enemigo: no puede hacer nada.         */
    LQ_LANZADO,                      /* Volando por un lanzamiento, hasta aterrizar.       */
    LQ_RECIBIR,                      /* Retrocede por un golpe.                            */
    LQ_DERRIBADO,                    /* Volando por un derribo, hasta aterrizar.           */
    LQ_EN_SUELO,                     /* Tirado en el suelo.                                */
    LQ_LEVANTARSE,                   /* Se levanta: invulnerable mientras tanto.           */
    LQ_KO,                           /* Vencido: cae y, al acabar el tiempo, sale.         */
    LQ_ENTRADA,                      /* Relevo: cae desde arriba, invulnerable.            */
    LQ_ENTRADA_GOLPE,                /* Relevo: aterrizó; el golpe de entrada, un instante. */
    LQ_ESTADOS                       /* Cuántos estados hay (tamaño de las tablas).        */
};

typedef struct
{
    Cuerpo c;                        /* Posición, velocidad y altura.                      */
    Golpeable g;                     /* Vida y contadores de combate.                      */
    DobleToque d;                    /* Doble toque para correr.                           */
    u8 estado;                       /* Un valor de LuchadorEstado.                        */
    u8 tiempo;                       /* Frames en el estado actual.                        */
    u8 encadenar;                    /* 1 si se pulsó A en la ventana de encadenado.       */
    u8 conecto;                      /* 1 si el estado actual ya le dio a alguien.         */
    u8 pagado;                       /* 1 si el especial ya pagó su coste.                 */
    u8 golpes_agarre;                /* Golpes de agarre dados (máximo 3).                 */
    u8 mira_izquierda;               /* 1 si mira a la izquierda.                          */
    u8 vencido;                      /* 1 cuando el KO ha terminado.                       */
    u8 quien;                        /* QUIEN_TAROU o QUIEN_BARBARA.                       */
} Luchador;

/* Lo deja de pie, quieto, en (x, y) del mundo, con `vida` puntos: Tarou. */
void luchador_iniciar(Luchador *L, u16 x, u16 y, u8 vida);

/* Lo mismo para `quien` (QUIEN_TAROU o QUIEN_BARBARA). */
void luchador_iniciar_como(Luchador *L, u8 quien, u16 x, u16 y, u8 vida);

/* La celda de dibujo del luchador (Tarou 96x64; Bárbara 96x96), para pasar las cajas al mundo. */
void luchador_celda(const Luchador *L, Celda *celda);

/* Relevo: lo hace entrar cayendo desde ALTURA_ENTRADA_RELEVO px sobre (x_fp, y_fp), invulnerable
   `invulnerable` frames. Al aterrizar da el golpe de entrada (ver luchador_entrada_activa). */
void luchador_entrar(Luchador *L, s16 x_fp, s16 y_fp, u8 invulnerable);

/* 1 solo en el frame en que el que entra aterriza: ese frame se resuelve el golpe de entrada. */
u8 luchador_entrada_activa(const Luchador *L);

/* 1 si puede salir en un relevo: no está recibiendo, derribado, agarrado, KO ni entrando. */
u8 luchador_puede_salir(const Luchador *L);

/* Un frame: lee el mando, cambia de estado, mueve el cuerpo y avanza los contadores.
   Si está congelado por una parada de impacto, no hace nada salvo bajar el contador. */
void luchador_actualizar(Luchador *L, const Mando *m, const Carril *carril);

/* El golpe actual le ha dado a alguien (el llamador lo sabe por combate_golpear). */
void luchador_marcar_conecto(Luchador *L);

/* Reacciona al resultado de combate_golpear sobre este luchador (GOLPE_*). */
void luchador_reaccionar(Luchador *L, u8 resultado);

/* La caja de golpe de este frame, o NULL si no hay ninguna activa. */
const CajaCuadro *luchador_caja_golpe(const Luchador *L);

/* Empieza un agarre (lo llama quien agarra) o lo suelta (quien agarra o el agarrado). */
void luchador_agarrar(Luchador *L);
void luchador_soltar(Luchador *L);

/* Lo agarra un enemigo: queda AGARRADO hasta que se suelte. */
void luchador_ser_agarrado(Luchador *L);

/* Lo lanza un agarre: vuela con esas velocidades (punto fijo; vz > 0 sube). */
void luchador_lanzar(Luchador *L, s16 vx_fp, s16 vz_fp);

#endif /* LUCHADOR_H */
