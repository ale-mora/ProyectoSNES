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
      el golpe de Tarou llama a ninja_recibir. Qué dibujo de «Ninja 1» toca lo dice ninja_dibujo().
      Hay NINJAS_MAX ranuras fijas: la oleada rellena las libres. El dibujo es de
      enemigos/ninja_sprite.c: cada ranura tiene su hueco de tiles en la VRAM.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Lo usan:  enemigos/ninja_sprite.c, pantallas/pantalla_fase1.c y tests/prueba_ninja.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (campos de Ninja; 32 bytes: potencia de 2)
      x, y        u16  pies: x en el mundo, y línea de pantalla del suelo
      vivo        u8   1 si la ranura está en uso
      mira_izq    u8   1 si mira a la izquierda (hacia el protagonista)
      cuadro      u8   cuadro de la tira de caminar (0..NINJA_CUADROS-1)
      reloj       u8   frames que lleva en ese cuadro; también reparte el paso en y
      estado      u8   NINJA_PARADO o NINJA_ANDANDO
      dentro      u8   1 desde que entró en la pantalla (a partir de ahí ya no puede salir)
      vida        u8   puntos de vida (NINJA_VIDA al nacer; a 0, cae y desaparece)
      id_golpe    u8   último tajo que le dio (juego/mundo.h: Cuerpo.id): un tajo no pega dos veces
      timer       u8   cuadros que le quedan en RECIBIR / DERRIBADO
      empuje      u8   hacia dónde lo empujó el golpe: NINJA_EMPUJE_IZQ o NINJA_EMPUJE_DER
      espera      u8   descanso que le queda antes del próximo ataque
      conecto     u8   1 si el tajo en curso ya acertó
      tipo        u8   NINJA_NORMAL / _MEDIANO / _GRANDE: su tamaño, color, vida y daño
    → glosario: Oleada, IA, Carril
---------------------------------------------------------------------------------*/
#ifndef NINJA_H
#define NINJA_H

#include "sistema/tipos.h"

#define NINJAS_MAX 3                     /* Ranuras (las que caben en la VRAM de sprites).  */
#define NINJA_CUADROS 6                  /* Dibujos de caminar de «Ninja 1» (cuadro 0..5).  */
#define NINJA_FRAMES_CUADRO 4            /* Cada cuántos frames cambia de cuadro.           */
#define NINJA_PASO_X 1                   /* Anda más lento que Tarou (2): se le puede huir. */
#define NINJA_DISTANCIA 40               /* Se detiene a esta distancia en x del objetivo.  */
#define NINJA_VIDA 4                     /* El normal: combo de Tarou 1 + 1 + 2 = justo.    */
#define NINJA_RECIBIR_FRAMES 16          /* Aturdido tras un golpe normal.                  */
#define NINJA_LANZADO_FRAMES 18           /* Lanzado: vuela estos frames (5 px/frame)...     */
#define NINJA_LANZADO_PASO 5
#define NINJA_GOLPE_LANZADO 255          /* ...y su choque cuenta como este tajo.           */
#define NINJA_DESTELLO_FRAMES 3          /* En blanco al recibir un golpe (game feel).      */
#define NINJA_INV_LEVANTARSE 40          /* Invulnerable (y parpadeando) tras levantarse.   */
#define NINJA_DERRIBO_FRAMES 64          /* Caído: cae (21), en el suelo (20) y se levanta (20);
                                            sin vida: KO (21) y parpadea tumbado hasta irse. */
#define NINJA_EMPUJE_IZQ 1
#define NINJA_EMPUJE_DER 2
/*--- El ataque del ninja (un puñetazo; cifras a ajustar jugando) ---------------------*/
#define NINJA_ATAQUE_FRAMES 28           /* Carga, puño fuera (12..19) y vuelta.            */
#define NINJA_GOLPE_DESDE 12             /* Su caja de golpe, activa de aquí...             */
#define NINJA_GOLPE_HASTA 20             /* ...hasta aquí (sin incluir): 8 cuadros.         */
#define NINJA_ALCANCE 48                 /* Hasta dónde llega por delante de sus pies.      */
#define NINJA_PROFUNDIDAD 12             /* Tolerancia en y para acertar.                   */
#define NINJA_DANIO 2                    /* Lo que quita el normal (Tarou tiene 24).        */
#define NINJA_ESPERA 50                  /* Cuadros de descanso entre un ataque y el siguiente. */
#define NINJA_ESPERA_INICIAL 30          /* Al nacer espera esto, +15 por ranura.           */
#define NINJA_TURNOS 2                   /* Como mucho dos atacan a la vez (§6 de la spec). */
#define NINJA_ENTRADA 40                 /* Aparece a esta distancia fuera del borde.       */
#define NINJA_ANCHO 96                   /* Celda de 96x96 de «Ninja 1» (tumbado mide 88 px). */
#define NINJA_ALTO 96

/*--- Los dibujos de «Ninja 1»: un número por dibujo, tira tras tira -----------------
    ninja_dibujo() da cuál toca; enemigos/ninja_sprite.c lo traduce a tira y cuadro. Las
    tiras van en el orden de ninja1_p16_bancos (data.asm). «En el suelo» es el último
    dibujo de derribado. */
#define NINJA_DIB_GUARDIA 0              /* 8 dibujos.                                      */
#define NINJA_DIB_CAMINAR 8              /* 6.                                              */
#define NINJA_DIB_ATAQUE 14              /* 3 (el puñetazo).                                */
#define NINJA_DIB_RECIBIR 17             /* 6.                                              */
#define NINJA_DIB_DERRIBADO 23           /* 7 (el último: en el suelo).                     */
#define NINJA_DIB_LEVANTARSE 30          /* 5.                                              */
#define NINJA_DIB_KO 35                  /* 7.                                              */
#define NINJA_DIB_AGARRADO 42           /* 2. Desde aquí, 2026-11-05 (los tipos sin esa   */
#define NINJA_DIB_BLOQUEO 44             /* 2.  tira usan otra: ninja_sprite.c).            */
#define NINJA_DIB_SHURIKEN 46            /* 8 (el rápido lanza un shuriken).                */
#define NINJA_DIB_AGARRE 54              /* 2 (agarra al protagonista).                     */
#define NINJA_DIB_LANZAR 56              /* 6 (lo lanza).                                   */
#define NINJA_DIB_PATADA 62              /* 7.                                              */
#define NINJA_DIB_CARGA 69               /* 6 (el jefe embiste).                            */
#define NINJA_DIB_CONVOCAR 75            /* 4 (el jefe llama a un ninja).                   */
#define NINJA_DIB_PESADO 79              /* 6 (el golpe del kendoka; el tajo del jefe).     */
#define NINJA_DIBUJOS 85

/*--- Las acciones nuevas (2026-11-05; cifras a ajustar jugando) ----------------------*/
#define NINJA_PATADA_FRAMES 32           /* La patada: activa de 14 a 22, llega a 56 px.   */
#define NINJA_PATADA_DESDE 14
#define NINJA_PATADA_HASTA 22
#define NINJA_PATADA_ALCANCE 56
#define NINJA_PESADO_FRAMES 36           /* Kendoka y jefe: activa de 16 a 24, 60 px, derriba. */
#define NINJA_PESADO_DESDE 16
#define NINJA_PESADO_HASTA 24
#define NINJA_PESADO_ALCANCE 60
#define NINJA_BLOQUEO_FRAMES 16          /* Bloqueando: los golpes de frente rebotan.       */
#define NINJA_AGARRE_FRAMES 36           /* Tiene agarrado al protagonista...               */
#define NINJA_LANZAR_FRAMES 24           /* ...y lo lanza (en el frame NINJA_LANZAR_SUELTA). */
#define NINJA_LANZAR_SUELTA 10
#define NINJA_AGARRE_ALCANCE 36
#define NINJA_DISPARO_FRAMES 32          /* El rápido lanza su shuriken (en el frame 18).   */
#define NINJA_DISPARO_SUELTA 18
#define NINJA_RAPIDO_CERCA 80            /* El rápido se mantiene entre estas distancias.   */
#define NINJA_RAPIDO_LEJOS 112
#define NINJA_CARGA_PASO 4               /* El jefe embiste a 4 px por frame...             */
#define NINJA_CARGA_FRAMES 48            /* ...como mucho estos frames.                     */
#define NINJA_CARGA_DESDE 110            /* Embiste si el protagonista está más lejos.      */
#define NINJA_CONVOCAR_FRAMES 16         /* Llama a un ninja (en el frame 8)...             */
#define NINJA_CONVOCA_CADA 150           /* ...cada 300 frames (cuenta de 2 en 2)...        */
#define NINJA_JEFE_FURIA 20              /* ...cuando le queda esta vida o menos.           */

typedef enum
{
    NINJA_EVENTO_NINGUNO = 0,
    NINJA_EVENTO_SHURIKEN,               /* Sale su shuriken (objetos.c).                   */
    NINJA_EVENTO_CONVOCAR,               /* El jefe llama a un ninja normal.                */
    NINJA_EVENTO_LANZA_PRESA             /* Lanza al protagonista que tenía agarrado.       */
} EventoNinja;

typedef enum
{
    NINJA_ATQ_PUNO = 0,
    NINJA_ATQ_PATADA,
    NINJA_ATQ_PESADO
} AtaqueNinja;

/*--- Los tres tipos (2026-11-04): uno por ranura, cada uno con su tamaño y su color -----
    La ranura k nace siempre como el tipo k, así que la oleada de la zona 0 trae un
    normal, la de la zona 1 un normal y un mediano, y desde la 2 los tres. */
typedef enum
{
    NINJA_NORMAL = 0,                    /* ~80 px, rojo: vida 4, daño 2.                   */
    NINJA_MEDIANO,                       /* ~90 px, amarillo: vida 6, daño 3.               */
    NINJA_GRANDE,                        /* ~100 px, morado: vida 8, daño 4, más lento.     */
    NINJA_RAPIDO,                        /* El normal en verde: guarda distancia y lanza shuriken. */
    NINJA_KENDOKA,                       /* Pesado: lento, bloquea de frente, agarra y lanza. */
    NINJA_JEFE,                          /* Gomasuri-dono: embiste, tajo largo, llama ninjas. */
    NINJA_TIPOS
} TipoNinja;

typedef enum
{
    NINJA_PARADO = 0,
    NINJA_ANDANDO,
    NINJA_ATACANDO,                      /* Dando un tajo (su caja está activa en parte de él). */
    NINJA_RECIBIR,                       /* Aturdido: retrocede un poco y no hace nada.     */
    NINJA_DERRIBADO,                     /* Derribado: sale despedido y cae; sin vida, desaparece. */
    NINJA_AGARRADO,                      /* Lo tiene agarrado el protagonista (combate.c lo coloca). */
    NINJA_BLOQUEO,                       /* Desde aquí, 2026-11-05.                         */
    NINJA_AGARRANDO,                     /* Tiene agarrado al protagonista.                 */
    NINJA_LANZANDO,                      /* Lo lanza.                                       */
    NINJA_DISPARANDO,                    /* Lanza un shuriken (el rápido).                  */
    NINJA_CARGANDO,                      /* Embiste (el jefe).                              */
    NINJA_CONVOCANDO                     /* Llama a un ninja (el jefe).                     */
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
    u8 tipo;                             /* TipoNinja (= su ranura).                        */
    u8 destello;                         /* Frames que le quedan en blanco (acaba de recibir). */
    u8 inv;                              /* Frames invulnerable tras levantarse (parpadea). */
    u8 lanzado;                          /* 1 = lo lanzaron: vuela y derriba a los que toca. */
    u8 ataque;                           /* AtaqueNinja del ataque en curso.                */
    u8 evento;                           /* EventoNinja de este frame (dura uno).           */
    u8 convoca;                          /* El jefe: cuenta atrás para llamar a otro ninja. */
    u8 dados;                            /* Ataques que lleva (alterna puño y patada).      */
    u8 vio;                              /* Último golpe del protagonista que vio venir.    */
    u8 libre[7];                        /* Relleno hasta 32 bytes (potencia de 2: n + k sin tcc__mul). */
} Ninja;

/* Deja las NINJAS_MAX ranuras libres (y la semilla del azar, fija: se repite igual). */
void ninja_iniciar_todos(Ninja *n);

/* Una oleada con estos tipos: cada uno en una ranura libre que admita su tamaño (la 0, de
   16 piezas, solo el normal y el rápido; el jefe ocupa la 1 y deja la 2 vacía), por la
   izquierda y la derecha alternando. Devuelve cuántos nacieron. */
u8 ninja_oleada_tipos(Ninja *n, u16 camara, const u8 *tipos, u8 cuantos);

/* Lo que el combate le cuenta a la IA cada frame: si al protagonista se le puede agarrar
   (de pie, en el suelo y sin invulnerabilidad) y el golpe que está dando (su id, 0 si
   ninguno), x e y y hacia dónde mira, para que el ninja normal decida bloquear. */
void ninja_contexto(u8 presa_libre, u8 golpe_id, u16 x, u16 y, u8 mira_izq);

/* ¿Bloquea este golpe? (frontal, no `rompe`: los aéreos, el especial y los lanzados
   rompen la guardia). El kendoka bloquea siempre de frente si no está atacando; el
   normal, si estaba bloqueando. Si bloquea, apunta el id para no volver a mirarlo. */
u8 ninja_bloquea(Ninja *n, u8 hacia_izq, u8 rompe, u8 id);

/* Alcance del golpe en curso y si derriba (el pesado y la embestida). */
u16 ninja_alcance(const Ninja *n);
u8 ninja_derriba(const Ninja *n);

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

/* 1 si se le puede agarrar: vivo, dentro, quieto, andando o aturdido, y no recién levantado. */
u8 ninja_agarrable(const Ninja *n);

/* Lo agarran (queda en NINJA_AGARRADO hasta que lo suelten o lo lancen). */
void ninja_agarrar(Ninja *n);

/* Lo sueltan: con `derribo`, cae (tras los golpes de agarre); sin él, vuelve a la carga. */
void ninja_soltar(Ninja *n, u8 derribo, u8 hacia_izq);

/* Lo lanzan hacia la izquierda (`hacia_izq` 1) o la derecha: pierde `danio` y vuela
   NINJA_LANZADO_FRAMES; mientras vuela, derriba a los que toca (combate.c). */
void ninja_lanzar(Ninja *n, u8 hacia_izq, u8 danio);

/* 1 si está volando tras un lanzamiento. */
u8 ninja_volando(const Ninja *n);

/* ¿Cuántos están dando un tajo? (reparte los turnos de ataque.) */
u8 ninja_atacando(const Ninja *n);

/* Tras ninja_actualizar, si el ninja está parado cerca del objetivo (x e y), descansado y
   `turno_libre` vale 1: empieza un tajo y devuelve 1. */
u8 ninja_intentar_atacar(Ninja *n, u16 obj_x, u16 obj_y, u8 turno_libre);

/* 1 si el tajo está en los cuadros en que su caja de golpe es activa y aún no ha acertado. */
u8 ninja_golpe_activo(const Ninja *n);

/* Qué dibujo de «Ninja 1» toca (NINJA_DIB_*): guardia en bucle parado, caminar andando,
   el puñetazo repartido en el ataque (puño fuera mientras golpea), recibir, y al caer
   derribado → en el suelo → levantarse, o, sin vida, el KO. */
u8 ninja_dibujo(const Ninja *n);

/* 1 si este frame el cuerpo no se dibuja: el ninja KO parpadea tumbado antes de irse, y
   el que se acaba de levantar parpadea mientras es invulnerable. */
u8 ninja_parpadea(const Ninja *n);

/* Lo que quita su golpe al protagonista: 2, 3 o 4 según el tipo. */
u8 ninja_danio(const Ninja *n);

/* Cuántos están vivos. */
u8 ninja_vivos(const Ninja *n);

/* Los quita a todos (provisional: «oleada vencida»; luego, el combate los irá derrotando). */
void ninja_derrotar_todos(Ninja *n);

#endif /* NINJA_H */
