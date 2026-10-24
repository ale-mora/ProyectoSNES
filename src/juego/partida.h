/*---------------------------------------------------------------------------------
    partida.h — Las reglas del tutorial: caer, pincharse, puntos de control, el
    tronco y la salida.

    PROPÓSITO
      Tarou se mueve solo (juego/tarou.c); este módulo mira dónde está después
      de moverse y decide qué le pasa a la PARTIDA:
        - Si cae por un foso (la celda baja de la fila 192) o toca una zarza,
          hay que REAPARECER en el último punto de control.
        - Cada cartel es un punto de control: al pasar por él, se apunta.
        - El tronco es un bloque sólido (juego/colision.c) hasta que lo toca la
          caja de golpe de la espada: entonces se ROMPE y desaparece.
        - Al llegar a la salida, el nivel termina.
      Sin hardware: lógica pura, probada en el PC (tests/prueba_partida.c). La
      pantalla del nivel hace lo que toca con cada evento (fundidos, redibujar
      columnas, cambiar de pantalla).

    SE CONECTA CON
      Incluye:  juego/tarou.h, sistema/tipos.h.
      Usa:      juego/colision.h (el bloque del tronco, las zarzas).
      Lo usan:  pantallas/pantalla_nivel1.c, tests/prueba_partida.c.
      Datos:    las x de inicio, controles, tronco y salida vienen de
                nivel1_datos.h (scripts/preparar_nivel.py, desde Tiled).

    HARDWARE QUE TOCA
      ninguno: lógica pura (se compila también en el PC).

    VARIABLES (constantes)
      PARTIDA_SIGUE, PARTIDA_REAPARECER, PARTIDA_TRONCO_ROTO, PARTIDA_SALIDA
                         los eventos que devuelve partida_actualizar
      TRONCO_ANCHO 32, TRONCO_ALTO 96   px: el tronco (más alto que un salto, 69 px)
      Y_CAIDA      192   px: con la celda más abajo, Tarou ya cayó del todo

    → glosario: Punto de control, Estado
---------------------------------------------------------------------------------*/
#ifndef PARTIDA_H
#define PARTIDA_H

#include "sistema/tipos.h"
#include "juego/tarou.h"

#define PARTIDA_SIGUE 0          /* Nada especial este frame.                              */
#define PARTIDA_REAPARECER 1     /* Cayó o se pinchó: llevarlo al último punto de control. */
#define PARTIDA_TRONCO_ROTO 2    /* La espada acaba de romper el tronco.                   */
#define PARTIDA_SALIDA 3         /* Llegó a la salida: fin del nivel.                      */

#define TRONCO_ANCHO 32          /* El tronco: 32 px de ancho (4 columnas de tiles)...     */
#define TRONCO_ALTO 96           /* ...y 96 de alto: ni con el salto largo (69 px) se pasa. */
#define Y_CAIDA 192              /* Celda en la 192: los pies en la 256, fuera de pantalla. */

typedef struct
{
    const u16 *controles;        /* x (centro) de cada punto de control, en orden.         */
    u8 n_controles;              /* Cuántos hay.                                            */
    u16 reaparicion;             /* x (centro) donde se reaparece: el último pasado.        */
    u8 siguiente;                /* Índice del próximo punto de control por pasar.          */
    u16 tronco_x;                /* Borde izquierdo del tronco (0 = no hay tronco).         */
    u8 tronco_entero;            /* 1 hasta que la espada lo rompe.                         */
    u16 salida_x;                /* Con el centro de Tarou aquí o más allá, fin del nivel.  */
} Partida;

/* Prepara la partida y pone el tronco como bloque sólido en la colisión (llamar
   después de colision_usar, que borra el bloque). inicio_x es el centro de Tarou. */
void partida_iniciar(Partida *p, u16 inicio_x, const u16 *controles, u8 n_controles,
                     u16 tronco_x, u16 salida_x);

/* Un frame de reglas, después de tarou_actualizar. Devuelve un PARTIDA_*. */
u8 partida_actualizar(Partida *p, const Tarou *t);

#endif /* PARTIDA_H */
