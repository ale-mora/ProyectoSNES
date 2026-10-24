/*---------------------------------------------------------------------------------
    fase.h — El director de la fase 1: une relevo, apoyos, oleadas, enemigos y
    marcador en una sola partida (spec §3-§7).

    PROPÓSITO
      Es el cerebro de la pantalla de la fase, pero SIN hardware: cada frame recibe el
      mando y mueve a todos. Cuida de los enemigos (hasta 3 a la vez, repartidos en
      profundidad; como mucho 2 atacan a la vez), resuelve los golpes en los dos sentidos
      con combate.c, el golpe de entrada del relevo y los apoyos (la acción se congela
      mientras actúan), suma puntos, lleva la cámara (sigue al protagonista, no retrocede
      y se bloquea con las oleadas), y encadena las zonas, las escenas, el STAGE CLEAR y el
      fin de la partida. No dibuja nada: devuelve EVENTOS que la pantalla aplica
      (cargar una escena, cambiar de zona...). Por eso se puede probar entera en el PC.
      PLACEHOLDERS conocidos: los tipos RÁPIDO y PESADO se comportan como el ninja básico
      (solo cambian vida y daño por el rango), y el jefe es un ninja de 160 de vida: no
      existen aún sus ataques propios (carga, tajo de alcance, convocar).

    SE CONECTA CON
      Incluye:  sistema/tipos.h, juego/cambio_protagonista.h (relevo), juego/apoyos.h,
                juego/marcador.h, juego/oleadas.h, juego/fase1.h, enemigos/ninja.h,
                juego/combate.h, juego/mando.h.
      Lo usan:  tests/prueba_fase.c y pantallas/pantalla_fase.c.

    HARDWARE QUE TOCA
      ninguno (lógica pura, probada en el PC).

    VARIABLES (campos de Fase; la pantalla los LEE)
      relevo         Relevo       los dos protagonistas, el activo y el relevo
      ninjas[3]      Ninja        los enemigos (los huecos libres no cuentan)
      vivo[3]        u8           1 si el hueco tiene un enemigo
      jefe[3]        u8           1 si ese enemigo es el jefe
      contado[3]     u8           1 si ya dio sus puntos
      golpeados_p[3], golpeados_n[3]  u8   a quién ya le dio cada ataque (protagonista / ninja)
      apoyos         Apoyos       la barra y los apoyos
      marcador       Marcador     puntos, vidas y continues
      oleadas        Oleadas      la oleada en curso y el bloqueo de cámara
      zona           u8           ZONA_*
      estado         u8           FASE_*
      camara         u16          x de la cámara (px)
      escena         u8           la escena en marcha (ESC_*)
      apoyo          u8           el apoyo en marcha (APOYO_*)
      eventos_apoyo  u8           los eventos del apoyo este frame (para sus efectos)
      zona_completa  u8           1 cuando se vencieron todas las oleadas de la zona

    → glosario: Director de fase, Oleada, Relevo, Apoyo, Bloqueo de cámara
---------------------------------------------------------------------------------*/
#ifndef FASE_H
#define FASE_H

#include "sistema/tipos.h"
#include "juego/cambio_protagonista.h"
#include "juego/apoyos.h"
#include "juego/marcador.h"
#include "juego/oleadas.h"
#include "juego/fase1.h"
#include "juego/combate.h"
#include "juego/mando.h"
#include "enemigos/ninja.h"

#define FASE_ENEMIGOS 3              /* Decisión 36: 3 a la vez como máximo.               */
#define MAX_ATACANDO 2               /* Turnos de ataque (§6): 2 atacan a la vez.          */
#define X_INICIAL_ZONA 64            /* Dónde empieza el protagonista en cada zona (px).   */
#define SALIDA_MARGEN 32             /* Faltan 32 px para el final de la zona: salida.     */
#define CAMARA_ADELANTE 120          /* El protagonista queda 120 px a la derecha del borde. */

/* Qué está pasando en la fase. */
enum
{
    FASE_JUGANDO = 0,                /* Se juega.                                          */
    FASE_ESCENA,                     /* Una escena en marcha: la acción está parada.       */
    FASE_APOYO,                      /* Un apoyo actúa: la acción está parada.             */
    FASE_SALIDA,                     /* Se acabó la zona: la pantalla cambia a la siguiente. */
    FASE_CLEAR,                      /* STAGE CLEAR.                                       */
    FASE_FIN_JUEGO                   /* Sin vidas: continue o título.                      */
};

/* Eventos de un frame (se suman en una máscara). */
enum
{
    FASE_EV_NADA = 0,
    FASE_EV_ESCENA = 1,              /* Jugar la escena F->escena y llamar a fase_escena_terminada. */
    FASE_EV_SALIDA = 2,              /* Cargar la zona siguiente y llamar a fase_siguiente_zona.    */
    FASE_EV_CLEAR = 4,               /* Mostrar STAGE CLEAR.                                        */
    FASE_EV_FIN_JUEGO = 8,           /* Sin vidas: ofrecer el continue (fase_continuar).            */
    FASE_EV_APOYO = 16,              /* Empezó un apoyo (F->apoyo): sus efectos de pantalla.        */
    FASE_EV_VIDA_PERDIDA = 32        /* Cayeron los dos: perdió una vida y reaparecen.              */
};

typedef struct
{
    Relevo relevo;
    Ninja ninjas[FASE_ENEMIGOS];
    u8 vivo[FASE_ENEMIGOS];
    u8 jefe[FASE_ENEMIGOS];
    u8 contado[FASE_ENEMIGOS];
    u8 golpeados_p[FASE_ENEMIGOS];
    u8 golpeados_n[FASE_ENEMIGOS];
    Apoyos apoyos;
    Marcador marcador;
    Oleadas oleadas;
    u8 zona;
    u8 estado;
    u16 camara;
    u8 escena;
    u8 apoyo;
    u8 eventos_apoyo;
    u8 zona_completa;
} Fase;

/* Empieza la partida: zona 0, Tarou activo, 3 vidas, sin apoyos ni barra. */
void fase_iniciar(Fase *f);

/* Un frame con el mando. Devuelve la máscara de FASE_EV_*. */
u8 fase_actualizar(Fase *f, const Mando *m);

/* La escena en marcha acabó: aplica lo que cambia (Lucinda y Rex se unen; la barra se activa) y
   sigue la fase. Devuelve eventos (FASE_EV_CLEAR si era la huida del villano en el dojo). */
u8 fase_escena_terminada(Fase *f);

/* La pantalla cargó la zona siguiente: la fase la empieza (cámara a 0, protagonistas al inicio). */
void fase_siguiente_zona(Fase *f);

/* Continue: si quedan, vuelve con las vidas de inicio y devuelve 1; si no, 0 (título). */
u8 fase_continuar(Fase *f);

/* Cuántos enemigos hay en pie ahora. */
u8 fase_vivos(const Fase *f);

#endif /* FASE_H */
