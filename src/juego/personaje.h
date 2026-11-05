/*---------------------------------------------------------------------------------
    personaje.h — Quién es la protagonista (Tarou o Bárbara) y qué animación toca.

    PROPÓSITO
      El selector (pantallas/pantalla_selector.c) deja en personaje_elegido a quién
      se juega en la fase 1; el resto del juego lo lee de aquí. Cada personaje tiene
      su tabla de animaciones (AnimProta: qué tira, cuántos cuadros, cada cuántos
      frames cambia y si hace bucle), su celda y el alcance de su golpe. Las dos
      tablas tienen las mismas entradas (enum AnimProtaId), así que la lógica que
      elige la animación a partir del Cuerpo (mundo.h) es la misma para las dos.
      Lógica pura, sin snes.h: se prueba en el PC (tests/prueba_personaje.c).

    SE CONECTA CON
      Incluye:  sistema/tipos.h, juego/mundo.h.
      Implementa: juego/personaje.c (con tarou_anim.h y barbara_anim.h, generados).
      Lo usan:  juego/protagonista_sprite.c (qué tira y cuadro subir),
                pantallas/pantalla_selector.c y pantallas/pantalla_fase1.c
                (personaje_elegido, alcance), tests/prueba_personaje.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES
      personaje_elegido  u8  PERSONAJE_TAROU / PERSONAJE_BARBARA (en personaje.c; main.c
                             lo pone a Tarou al arrancar: crt0 no lo deja a 0)
    → glosario: Estado, Tabla de animaciones
---------------------------------------------------------------------------------*/
#ifndef PERSONAJE_H
#define PERSONAJE_H

#include "sistema/tipos.h"
#include "juego/mundo.h"

typedef enum
{
    PERSONAJE_TAROU = 0,
    PERSONAJE_BARBARA,
    PERSONAJES                           /* Cuántos hay.                                  */
} Personaje;

/* Las animaciones que la fase 1 sabe enseñar (las mismas para los dos personajes). */
typedef enum
{
    ANIM_GUARDIA = 0,                    /* Quieta.                                       */
    ANIM_CAMINAR,
    ANIM_SALTAR,                         /* Subiendo.                                     */
    ANIM_CAER,                           /* Bajando.                                      */
    ANIM_GOLPE_1,                        /* Los tres golpes del combo.                    */
    ANIM_GOLPE_2,
    ANIM_GOLPE_3,
    ANIM_RECIBIR,                        /* Aturdida por un golpe.                        */
    ANIM_KO,                             /* Sin vida, en el suelo.                        */
    ANIM_PROTA_CANTIDAD
} AnimProtaId;

/* Las tiras de Bárbara, en el orden de barbara_p16_bancos (data.asm). */
typedef enum
{
    BARBARA_TIRA_GUARDIA = 0,
    BARBARA_TIRA_CAMINAR,
    BARBARA_TIRA_SALTAR,
    BARBARA_TIRA_CAER,
    BARBARA_TIRA_GOLPE_1,
    BARBARA_TIRA_GOLPE_2,
    BARBARA_TIRA_GOLPE_3,
    BARBARA_TIRA_RECIBIR,
    BARBARA_TIRA_KO,
    BARBARA_TIRAS
} BarbaraTira;

/* Las tiras de beat 'em up de Tarou, detrás de las del plataformas (enum Estado, 0..12) y
   su guardia fija (13) en tarou_p16_bancos (data.asm). */
#define TAROU_TIRA_GUARDIA_BU 14
#define TAROU_TIRA_GOLPE_1 15
#define TAROU_TIRA_GOLPE_2 16
#define TAROU_TIRA_GOLPE_3 17
#define TAROU_TIRA_RECIBIR 18
#define TAROU_TIRA_KO 19

/* Una animación: 4 bytes (potencia de 2: tabla[i] no llama a tcc__mul). */
typedef struct
{
    u8 tira;                             /* Tarou: su enum Estado (animaciones.h); Bárbara: BarbaraTira. */
    u8 cuadros;                          /* Cuántos cuadros tiene la tira.                */
    u8 frames;                           /* Frames que dura cada cuadro.                  */
    u8 bucle;                            /* 1 = vuelve al primero; 0 = se queda en el último. */
} AnimProta;

/* Por dónde va la animación que se enseña. */
typedef struct
{
    u8 anim;                             /* AnimProtaId.                                  */
    u8 cuadro;                           /* Cuadro dentro de la tira.                     */
    u8 reloj;                            /* Frames que lleva en ese cuadro.               */
    u8 libre;
} AnimEstado;

#define ALCANCE_TAROU 52                 /* Px por delante de los pies (combate.h).       */
#define ALCANCE_BARBARA 60               /* Hasta la punta de su katana (~55 px desde que mide ~70). */

extern u8 personaje_elegido;

/* La tabla de animaciones del personaje (ANIM_PROTA_CANTIDAD entradas). */
const AnimProta *personaje_anims(u8 personaje);

/* Ancho y alto de su celda (Tarou 96x64, Bárbara 112x96). */
u8 personaje_ancho(u8 personaje);
u8 personaje_alto(u8 personaje);

/* Alcance de su golpe en px (combate_resolver). */
u16 personaje_alcance(u8 personaje);

/* Qué animación toca según el cuerpo: el estado y, al atacar, el golpe del combo. */
u8 personaje_anim_de(const Cuerpo *c);

/* Un frame de la animación: si cambia, empieza por el cuadro 0; si no, cuenta frames y
   avanza (al final vuelve al primero o se queda en el último, según bucle). */
void personaje_animar(AnimEstado *e, const AnimProta *tabla, u8 anim);

#endif /* PERSONAJE_H */
