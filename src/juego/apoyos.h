/*---------------------------------------------------------------------------------
    apoyos.h — Los tres apoyos y la barra de ataque (spec §5.4 y decisiones 15-18).

    PROPÓSITO
      Cada golpe que conecta suma uno a la barra de ataque, que comparten los dos
      protagonistas; llena (BARRA_LLENA = 48 golpes) permite invocar el apoyo elegido
      (X) y vuelve a 0: un solo uso. R cambia al siguiente apoyo disponible. La barra
      está bloqueada hasta el monasterio de Rex (decisión 26) y cada apoyo se
      desbloquea cuando se une al grupo (Lucinda en el bosque, Rex en el monasterio,
      Fenrir en la fase 2). Si caen los dos protagonistas, vuelven con la barra llena.
      Al invocar, la acción se congela y el apoyo hace su ataque a toda la pantalla
      (spec §5.4); aquí se lleva su cronología, frame a frame, como eventos que la
      pantalla aplica: GOLPE (daño a todos), LEVANTAR (los enemigos suben, solo Rex),
      CURAR (cura al activo, solo Rex) y FIN.
      Contra los JEFES, el daño es la mitad y no derriban; los enemigos normales
      reciben el daño completo y caen derribados.
      Los tiempos de cada apoyo son una PROPUESTA (no estaban en la spec): se ajustan
      jugando. Lógica pura, probada en el PC.

    SE CONECTA CON
      Incluye:  sistema/tipos.h, juego/cifras.h.
      Lo usan:  tests/prueba_apoyos.c y, cuando exista, la pantalla de la fase.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (campos de Apoyos, 8 bytes)
      golpes        u8   0..BARRA_LLENA              golpes que conectaron desde el último apoyo
      disponibles   u8   bits 0..2                   apoyos que ya se han unido al grupo
      seleccionado  u8   APOYO_*                     el que invoca X
      activa        u8   0 / 1                       1 desde el monasterio (la barra empieza llena); antes no se llena
      en_curso      u8   0 = ninguno, si no apoyo+1  el apoyo que está actuando
      tiempo        u8   0..duración                 frames del apoyo en curso
      relleno       u8[2]

    → glosario: Apoyo, Barra de ataque
---------------------------------------------------------------------------------*/
#ifndef APOYOS_H
#define APOYOS_H

#include "sistema/tipos.h"
#include "juego/cifras.h"       /* BARRA_LLENA: los golpes que llenan la barra (también la lee el HUD). */

/* Los apoyos (el orden es el de la selección con R). */
enum { APOYO_LUCINDA = 0, APOYO_REX = 1, APOYO_FENRIR = 2, APOYOS = 3 };

/* Eventos de un frame del apoyo en curso (se suman en una máscara). */
enum
{
    APOYO_EV_NADA = 0,
    APOYO_EV_GOLPE = 1,              /* Daño a todos los enemigos en pantalla (ver apoyos_dano). */
    APOYO_EV_LEVANTAR = 2,           /* Rex: los enemigos suben (z) y quedan en el aire.   */
    APOYO_EV_CURAR = 4,              /* Rex: cura al protagonista activo (apoyos_curar_vida). */
    APOYO_EV_FIN = 8                 /* El apoyo acabó: se descongela la acción.           */
};

typedef struct
{
    u8 golpes;
    u8 disponibles;
    u8 seleccionado;
    u8 activa;
    u8 en_curso;
    u8 tiempo;
    u8 relleno[2];
} Apoyos;

/* Todo apagado: barra a 0, bloqueada, ningún apoyo todavía. */
void apoyos_iniciar(Apoyos *a);

/* El monasterio de Rex: la barra se activa y empieza LLENA (para probar el primer apoyo). Activarla
   otra vez no la vuelve a llenar. */
void apoyos_activar_barra(Apoyos *a);

/* Un apoyo se une al grupo. El primero que llega queda elegido. */
void apoyos_desbloquear(Apoyos *a, u8 apoyo);

/* Cae el último protagonista: vuelven con la barra llena. */
void apoyos_llenar(Apoyos *a);

/* Un golpe conectó: suma uno (si la barra está activa y no hay apoyo en marcha). Devuelve 1
   solo en el golpe que la llena. */
u8 apoyos_golpe_conecto(Apoyos *a);

/* R: pasa al siguiente apoyo disponible. */
void apoyos_siguiente(Apoyos *a);

/* 1 si X invocaría ahora: barra activa y llena, el elegido disponible y ningún apoyo en curso. */
u8 apoyos_puede_invocar(const Apoyos *a);

/* X: empieza el apoyo elegido y vacía la barra. Devuelve 1 si empezó (el apoyo es a->en_curso - 1). */
u8 apoyos_invocar(Apoyos *a);

/* Un frame del apoyo en curso: devuelve la máscara de APOYO_EV_*. Sin apoyo en curso, NADA. */
u8 apoyos_actualizar(Apoyos *a);

/* Daño de UN golpe del apoyo a un enemigo: normal (jefe = 0) o jefe (jefe = 1, la mitad). */
u8 apoyos_dano(u8 apoyo, u8 jefe);

/* 1 si el apoyo derriba a ese enemigo: a los normales sí, a los jefes no. */
u8 apoyos_derriba(u8 apoyo, u8 jefe);

/* La vida tras la cura de `apoyo` (solo Rex cura; el resto la deja igual), sin pasar de la máxima. */
u8 apoyos_curar_vida(u8 vida, u8 apoyo);

#endif /* APOYOS_H */
