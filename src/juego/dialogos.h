/*---------------------------------------------------------------------------------
    dialogos.h — Los textos en inglés de las escenas de la fase 1 (guion-historia.md §11).

    PROPÓSITO
      Cada escena es una lista de CUADROS; cada cuadro tiene quién habla y hasta dos
      líneas de texto. Los textos son los aprobados por el autor el 2026-10-23; cada
      línea cabe en 26 caracteres (lo que cabe junto al retrato) y no lleva acentos ni eñes
      (la fuente de 8x8 no los tiene). Los nombres propios van en mayúsculas. Es lógica
      pura: una tabla y cuatro consultas, probada en el PC.

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Lo usan:  pantallas/pantalla_fase.c y tests/prueba_dialogos.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna pública.
    → glosario: Diálogo, Escena
---------------------------------------------------------------------------------*/
#ifndef DIALOGOS_H
#define DIALOGOS_H

#include "sistema/tipos.h"

#define DIALOGO_LINEA_MAX 26         /* Caracteres por línea (junto al retrato).           */

/* Cuántos cuadros tiene la escena `escena` (ESC_*; 0 si no hay texto). */
u8 dialogo_cuadros(u8 escena);

/* Quién habla en el cuadro `n`, su primera línea y su segunda (cadena vacía si no hay). */
const char *dialogo_orador(u8 escena, u8 n);
const char *dialogo_linea1(u8 escena, u8 n);
const char *dialogo_linea2(u8 escena, u8 n);

#endif /* DIALOGOS_H */
