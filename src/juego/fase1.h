/*---------------------------------------------------------------------------------
    fase1.h — Los datos de la fase 1: las cuatro zonas, sus oleadas, sus objetos y
    sus escenas (spec §7 y decisiones del 2026-10-24).

    PROPÓSITO
      Es la tabla de la fase tal como la decidió el autor: aldea (1536 px), bosque de
      bambú (1280), monasterio de Rex (768) y dojo (1024), 4608 px en total. Cada zona
      lista sus OLEADAS (en qué x de la cámara salta cada una, qué enemigos y de qué
      rango, por qué lado entran, y la escena que va antes o después), sus ROMPIBLES
      (qué hay en cada uno) y su longitud. Mientras el script de Tiled (preparar_zona.py)
      no exista, estos datos viven aquí; cuando exista, saldrán de zonaN_datos.h con la
      misma forma, y estas pruebas seguirán valiendo.
      Las escenas dentro de la zona van «en mitad»: en el bosque, Lucinda se une al vencer
      la oleada 1 y Fenrir cruza como gag tras la 2; en el monasterio, la escena de Rex
      abre la zona (antes de su oleada); en el dojo, la entrada del jefe va antes de su
      oleada y la huida del villano, después. fase1_aplicar_escena traduce una escena a
      sus efectos sobre los apoyos (Lucinda y Rex se unen; la barra se activa).
      Lógica pura (datos y una función), probada en el PC.

    SE CONECTA CON
      Incluye:  sistema/tipos.h, juego/apoyos.h.
      Lo usan:  juego/oleadas.c y tests/prueba_oleadas.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (tipos que define)
      EnemigoDef (2 B)  tipo, rango            un enemigo de una oleada
      Oleada (16 B)     x, n, lado, e[3], escena_antes, escena_despues, relleno
      Rompible (4 B)    x, tipo, contiene      una cosa que se rompe y lo que suelta
      Zona              longitud, n_oleadas, n_rompibles, oleadas, rompibles

    → glosario: Fase, Zona, Oleada, Rango
---------------------------------------------------------------------------------*/
#ifndef FASE1_H
#define FASE1_H

#include "sistema/tipos.h"
#include "juego/apoyos.h"

#define FASE1_ZONAS 4                /* Aldea, bosque, monasterio, dojo.                   */
#define ZONA_ALDEA 0
#define ZONA_BOSQUE 1
#define ZONA_MONASTERIO 2
#define ZONA_DOJO 3

#define ANCHO_PANTALLA 256           /* La cámara muestra 256 px de la zona.               */
#define OLEADA_MAX_ENEMIGOS 3        /* Decisión 36: 3 como máximo a la vez.               */

/* Tipos de enemigo. El pesado siempre es de rango rojo (spec §5.5). */
enum { TIPO_BASICO = 0, TIPO_RAPIDO = 1, TIPO_PESADO = 2, TIPO_JEFE = 3 };

/* Por dónde entran los enemigos de una oleada. */
enum { LADO_DERECHA = 0, LADO_IZQUIERDA = 1, LADO_ALTERNO = 2 };

/* Escenas de la fase 1 (0 = ninguna). */
enum
{
    ESC_NADA = 0,
    ESC_LUCINDA = 1,                 /* Bosque, tras la oleada 1: Lucinda se une.          */
    ESC_FENRIR_GAG = 2,              /* Bosque, tras la oleada 2: Fenrir cruza corriendo.  */
    ESC_REX = 3,                     /* Monasterio, al abrir la zona: Rex se une y la barra se activa. */
    ESC_JEFE_ENTRA = 4,              /* Dojo, antes del jefe: su entrada con título.       */
    ESC_VILLANO_HUYE = 5             /* Dojo, tras el jefe: Nikoraemon huye con el juguete. */
};

/* Qué hay dentro de un rompible. */
enum { CONT_NADA = 0, CONT_ONIGIRI = 1, CONT_PESCADO = 2, CONT_SHURIKEN = 3, CONT_VIDA = 4 };

/* Qué es el rompible (decide su dibujo). */
enum { ROMP_BARRIL = 0, ROMP_CAJA = 1, ROMP_JARRA = 2, ROMP_FAROL = 3, ROMP_SHOJI = 4 };

typedef struct
{
    u8 tipo;                         /* TIPO_*.                                            */
    u8 rango;                        /* RANGO_* de cifras.h (0..3); en el jefe, sin uso.   */
} EnemigoDef;

typedef struct
{
    u16 x;                           /* x de la CÁMARA (px) a la que salta la oleada.      */
    u8 n;                            /* Cuántos enemigos (1..OLEADA_MAX_ENEMIGOS).         */
    u8 lado;                         /* LADO_*.                                            */
    EnemigoDef e[OLEADA_MAX_ENEMIGOS];
    u8 escena_antes;                 /* ESC_*: va antes de que salgan los enemigos.        */
    u8 escena_despues;               /* ESC_*: va tras vencer la oleada.                   */
    u8 relleno[4];                   /* Hasta 16 bytes.                                    */
} Oleada;

typedef struct
{
    u16 x;                           /* x del mundo (px) del rompible.                     */
    u8 tipo;                         /* ROMP_*.                                            */
    u8 contiene;                     /* CONT_*.                                            */
} Rompible;

typedef struct
{
    u16 longitud;                    /* Largo de la zona en px.                            */
    u8 n_oleadas;
    u8 n_rompibles;
    const Oleada *oleadas;
    const Rompible *rompibles;
} Zona;

/* La zona `id` (ZONA_*), o 0 si no existe. */
const Zona *fase1_zona(u8 id);

/* Aplica a los apoyos lo que cambia una escena: Lucinda se une (bosque); Rex se une y la barra se
   activa, llena (monasterio). El resto de escenas no cambia nada. */
void fase1_aplicar_escena(u8 escena, Apoyos *apoyos);

#endif /* FASE1_H */
