/*---------------------------------------------------------------------------------
    ninja.h — El ninja básico del beat 'em up: aparece, camina hacia el protagonista y se
    queda a distancia de ataque.

    PROPÓSITO
      Primer enemigo (docs/diseno-beat-em-up.md decisión 21: «ninja básico»). Es lógica
      pura, sin snes.h, probada en el PC (tests/prueba_ninja.c). Cada ninja tiene una
      posición en el carril (x, y de los pies, como el Cuerpo de juego/mundo.h), mira hacia
      el protagonista y camina hacia él: en x un píxel por frame hasta NINJA_DISTANCIA y en
      y un píxel cada dos frames hasta su carril. Cuando llega, descansado y con turno
      (como mucho NINJA_TURNOS a la vez), da un tajo (ATACANDO): su caja de golpe está activa de
      NINJA_GOLPE_DESDE a NINJA_GOLPE_HASTA y pega UNA vez; juego/combate.c decide si acierta, y
      el golpe de Tarou llama a ninja_recibir. El cuadro de dibujo es 0..7 andando y 8..14 atacando.
      Hay NINJAS_MAX ranuras fijas: la oleada rellena las libres. El dibujo es de
      enemigos/ninja_sprite.c: cada ranura tiene su hueco de tiles en la VRAM.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Lo usan:  enemigos/ninja_sprite.c, pantallas/pantalla_fase1.c y tests/prueba_ninja.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (campos de Ninja; 16 bytes: potencia de 2)
      x, y        u16  pies: x en el mundo, y línea de pantalla del suelo
      vivo        u8   1 si la ranura está en uso
      mira_izq    u8   1 si mira a la izquierda (hacia el protagonista)
      cuadro      u8   cuadro de la tira de andar (0..NINJA_CUADROS-1)
      reloj       u8   frames que lleva en ese cuadro; también reparte el paso en y
      estado      u8   NINJA_PARADO o NINJA_ANDANDO
      dentro      u8   1 desde que entró en la pantalla (a partir de ahí ya no puede salir)
      vida        u8   puntos de vida (NINJA_VIDA al nacer; a 0, cae y desaparece)
      id_golpe    u8   último tajo que le dio (juego/mundo.h: Cuerpo.id): un tajo no pega dos veces
      timer       u8   cuadros que le quedan en RECIBIR / DERRIBADO
      empuje      u8   hacia dónde lo empujó el golpe: NINJA_EMPUJE_IZQ o NINJA_EMPUJE_DER
      espera      u8   descanso que le queda antes del próximo ataque
      conecto     u8   1 si el tajo en curso ya acertó
    → glosario: Oleada, IA, Carril
---------------------------------------------------------------------------------*/
#ifndef NINJA_H
#define NINJA_H

#include "sistema/tipos.h"

#define NINJAS_MAX 3                     /* Ranuras (las que caben en la VRAM de sprites).  */
#define NINJA_CUADROS 8                  /* Cuadros de la tira de andar (cuadro 0..7).      */
#define NINJA_CUADROS_ATAQUE 7           /* Cuadros de la tira de ataque (cuadro 8..14).    */
#define NINJA_FRAMES_CUADRO 4            /* Cada cuántos frames cambia de cuadro.           */
#define NINJA_PASO_X 1                   /* Anda más lento que Tarou (2): se le puede huir. */
#define NINJA_DISTANCIA 40               /* Se detiene a esta distancia en x del objetivo.  */
#define NINJA_VIDA 4                     /* Combo de Tarou: 1 + 1 + 2 de daño = justo.      */
#define NINJA_RECIBIR_FRAMES 16          /* Aturdido tras un golpe normal.                  */
#define NINJA_DERRIBO_FRAMES 36          /* Caído (derribo, o el golpe que lo remata).      */
#define NINJA_EMPUJE_IZQ 1
#define NINJA_EMPUJE_DER 2
/*--- El ataque del ninja (un tajo de shinai; cifras a ajustar jugando) ---------------*/
#define NINJA_ATAQUE_FRAMES 28           /* Duración: 7 cuadros de arte x 4.                */
#define NINJA_GOLPE_DESDE 12             /* Su caja de golpe, activa de aquí...             */
#define NINJA_GOLPE_HASTA 20             /* ...hasta aquí (sin incluir): 8 cuadros.         */
#define NINJA_ALCANCE 48                 /* Hasta dónde llega por delante de sus pies.      */
#define NINJA_PROFUNDIDAD 12             /* Tolerancia en y para acertar.                   */
#define NINJA_DANIO 2                    /* Puntos de vida que quita (Tarou tiene 24).      */
#define NINJA_ESPERA 50                  /* Cuadros de descanso entre un ataque y el siguiente. */
#define NINJA_ESPERA_INICIAL 30          /* Al nacer espera esto, +15 por ranura.           */
#define NINJA_TURNOS 2                   /* Como mucho dos atacan a la vez (§6 de la spec). */
#define NINJA_ENTRADA 40                 /* Aparece a esta distancia fuera del borde.       */
#define NINJA_ANCHO 64                   /* Celda de 64x96 con el cuerpo en el centro.      */
#define NINJA_ALTO 96

typedef enum
{
    NINJA_PARADO = 0,
    NINJA_ANDANDO,
    NINJA_ATACANDO,                      /* Dando un tajo (su caja está activa en parte de él). */
    NINJA_RECIBIR,                       /* Aturdido: retrocede un poco y no hace nada.     */
    NINJA_DERRIBADO                      /* Derribado: sale despedido y cae; sin vida, desaparece. */
} EstadoNinja;

typedef struct
{
    u16 x;
    u16 y;
    u8 vivo;
    u8 mira_izq;
    u8 cuadro;
    u8 reloj;
    u8 estado;
    u8 dentro;
    u8 vida;
    u8 id_golpe;
    u8 timer;
    u8 empuje;
    u8 espera;                           /* Cuadros de descanso antes de poder atacar.      */
    u8 conecto;                          /* 1 si el tajo en curso ya acertó (no pega dos veces). */
} Ninja;

/* Deja las NINJAS_MAX ranuras libres. */
void ninja_iniciar_todos(Ninja *n);

/* Una oleada: ocupa hasta (zona + 1) ranuras libres, con un máximo de NINJAS_MAX, alternando
   por la izquierda y la derecha, fuera de la pantalla (cámara − NINJA_ENTRADA o cámara + 256 +
   NINJA_ENTRADA) y en carriles distintos. Devuelve cuántos nacieron. */
u8 ninja_oleada(Ninja *n, u16 camara, u8 zona);

/* Un frame de UN ninja vivo: mira y camina hacia (obj_x, obj_y), los pies del protagonista. */
void ninja_actualizar(Ninja *n, u16 obj_x, u16 obj_y);

/* Tras ninja_actualizar: una vez que el ninja ha entrado en [x_min, x_max] (la pantalla con
   un margen) ya no puede salir de ahí; mientras viene de fuera, no se toca. Así no se queda
   esperando fuera de la pantalla cuando el protagonista está pegado al borde. */
void ninja_limitar_x(Ninja *n, u16 x_min, u16 x_max);

/* El ninja recibe un golpe de `danio` puntos: pierde vida, se pone aturdido (o derribado si
   `derribo` vale 1 o se queda sin vida) y lo empuja hacia la izquierda si `hacia_izq` es 1 (si
   no, hacia la derecha), mirando hacia donde venía el golpe. `id` es el tajo: si ya lo recibió,
   no hace nada. Devuelve 1 si el golpe lo derribó o lo remató, 0 si solo lo aturdió o no contó. */
u8 ninja_recibir(Ninja *n, u8 danio, u8 derribo, u8 hacia_izq, u8 id);

/* ¿Cuántos están dando un tajo? (reparte los turnos de ataque.) */
u8 ninja_atacando(const Ninja *n);

/* Tras ninja_actualizar, si el ninja está parado cerca del objetivo (x e y), descansado y
   `turno_libre` vale 1: empieza un tajo y devuelve 1. */
u8 ninja_intentar_atacar(Ninja *n, u16 obj_x, u16 obj_y, u8 turno_libre);

/* 1 si el tajo está en los cuadros en que su caja de golpe es activa y aún no ha acertado. */
u8 ninja_golpe_activo(const Ninja *n);

/* Cuántos están vivos. */
u8 ninja_vivos(const Ninja *n);

/* Los quita a todos (provisional: «oleada vencida»; luego, el combate los irá derrotando). */
void ninja_derrotar_todos(Ninja *n);

#endif /* NINJA_H */
