/*---------------------------------------------------------------------------------
    ejercicio_07/marcador.h — Interfaz del marcador de puntos (ejercicio 7).

    PROPÓSITO
      Lo que main.c puede usar del marcador: el récord (una variable global,
      anunciada con extern) y tres funciones (sus prototipos). Este archivo
      ya está completo: léelo para saber qué tiene que hacer marcador.c.
      Tiene la forma de src/sistema/secuencia.h.

    SE CONECTA CON
      Incluye:  tipos.h.
      Lo usan:  ejercicio_07/marcador.c (lo implementa), ejercicio_07/main.c (lo usa).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (declaradas aquí, definidas en marcador.c)
      marcador_record  u16  0..65535  el total más alto alcanzado; sobrevive a
                                      marcador_iniciar (se conserva entre partidas)

    → conceptos (README de la unidad 00): .h, prototipo, extern
---------------------------------------------------------------------------------*/
#ifndef MARCADOR_H                   /* Guarda de inclusión.                              */
#define MARCADOR_H

#include "tipos.h"                   /* u8 y u16.                                         */

extern u16 marcador_record;          /* Vive en marcador.c; main.c la lee y la pone a 0.  */

void marcador_iniciar(void);         /* Empieza una partida: total a 0 (el récord, no).   */
u8 marcador_sumar(u16 puntos);       /* Suma puntos, actualiza el récord y devuelve cuántas
                                        sumas se llevan desde que arrancó el programa.     */
u16 marcador_total(void);            /* El total de la partida actual.                    */

#endif /* MARCADOR_H */
