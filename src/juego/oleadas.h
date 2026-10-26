/*---------------------------------------------------------------------------------
    oleadas.h — Las oleadas de una zona: cuándo salen, el bloqueo de la cámara y las
    escenas (spec §3 y §7, decisión 3).

    PROPÓSITO
      El avance es por zonas con oleadas: la cámara avanza hasta la x de la próxima
      oleada y SE BLOQUEA ahí; salen los enemigos de uno en uno cada
      FRAMES_ENTRE_ENEMIGOS frames; cuando han salido todos y no queda ninguno vivo,
      la oleada está vencida y aparece «GO →» (FRAMES_GO frames) para seguir. Una
      oleada puede llevar una escena ANTES (se juega antes de que salgan los
      enemigos) o DESPUÉS (al vencerla). Al vencer la última, la zona está completa.
      Este módulo no crea enemigos ni dibuja: cada frame recibe la x de la cámara y
      cuántos enemigos hay vivos, y devuelve eventos que quien llama aplica:
      SPAWN (sale un enemigo: lo que hay que crear está en spawn_*), ESCENA (hay que
      jugar la escena `escena`; al acabar, oleadas_escena_terminada), GO (flecha) y
      ZONA (completa). El tope de la cámara lo da oleadas_limite_camara.
      Lógica pura, probada en el PC.

    SE CONECTA CON
      Incluye:  sistema/tipos.h, juego/fase1.h.
      Lo usan:  tests/prueba_oleadas.c y, cuando exista, la pantalla de la fase.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (campos de Oleadas)
      zona         u8   ZONA_*                       la zona en curso
      oleada       u8   0..n_oleadas                 la oleada que toca o está en marcha
      estado       u8   OL_*                         qué está pasando
      aparecidos   u8   0..3                         cuántos enemigos de la oleada han salido
      espera       u8   frames                       hasta que salga el próximo
      go           u8   frames                       lo que queda de «GO →»
      escena       u8   ESC_*                        la escena pendiente (0 = ninguna)
      spawn_tipo, spawn_rango, spawn_lado  u8        el enemigo del último SPAWN
      spawn_x      u16  px del mundo                 por dónde entra

    → glosario: Oleada, Zona, Bloqueo de cámara
---------------------------------------------------------------------------------*/
#ifndef OLEADAS_H
#define OLEADAS_H

#include "sistema/tipos.h"
#include "juego/fase1.h"

#define FRAMES_ENTRE_ENEMIGOS 40     /* Un enemigo cada 40 frames dentro de una oleada.    */
#define FRAMES_GO 90                 /* Lo que dura el «GO →» (1,5 s).                     */
#define MARGEN_SPAWN 32              /* px fuera de la pantalla por los que entran.        */

/* Estados de la zona. */
enum
{
    OL_ESPERANDO = 0,                /* Avanzando hacia la próxima oleada.                 */
    OL_ESCENA_ANTES,                 /* Jugando la escena de antes de la oleada.           */
    OL_COMBATE,                      /* Cámara bloqueada: salen los enemigos y se les vence. */
    OL_ESCENA_DESPUES,               /* Jugando la escena de después de vencerla.          */
    OL_COMPLETA                      /* No quedan oleadas.                                 */
};

/* Eventos de un frame (se suman en una máscara). */
enum
{
    OL_EV_NADA = 0,
    OL_EV_SPAWN = 1,                 /* Sale un enemigo: ver spawn_tipo, spawn_rango, spawn_lado, spawn_x. */
    OL_EV_ESCENA = 2,                /* Hay que jugar la escena `escena`.                  */
    OL_EV_GO = 4,                    /* Oleada vencida: «GO →».                            */
    OL_EV_ZONA = 8                   /* Era la última: la zona está completa.              */
};

typedef struct
{
    u8 zona;
    u8 oleada;
    u8 estado;
    u8 aparecidos;
    u8 espera;
    u8 go;
    u8 escena;
    u8 spawn_tipo;
    u8 spawn_rango;
    u8 spawn_lado;
    u16 spawn_x;
} Oleadas;

/* Empieza la zona `zona` (ZONA_*): la primera oleada, sin nada en marcha. */
void oleadas_iniciar(Oleadas *o, u8 zona);

/* Un frame. `camara` es la x de la cámara en px; `vivos`, cuántos enemigos de la oleada siguen
   en pie. Devuelve la máscara de OL_EV_*. */
u8 oleadas_actualizar(Oleadas *o, u16 camara, u8 vivos);

/* La escena pendiente acabó. Devuelve los eventos que provoca (empezar a combatir no da ninguno;
   cerrar la oleada da GO, y ZONA si era la última). */
u8 oleadas_escena_terminada(Oleadas *o);

/* Hasta qué x puede avanzar la cámara ahora: la de la oleada en marcha o la próxima; al final, el
   fin de la zona (longitud - ANCHO_PANTALLA). */
u16 oleadas_limite_camara(const Oleadas *o);

/* 1 mientras se muestra «GO →». */
u8 oleadas_go_activo(const Oleadas *o);

#endif /* OLEADAS_H */
