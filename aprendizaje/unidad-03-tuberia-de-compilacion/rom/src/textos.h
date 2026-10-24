/*---------------------------------------------------------------------------------
    textos.h — Los textos y la posición del «hola mundo» de la unidad 03.

    PROPÓSITO
      Existe para enseñar una trampa de la tubería: snes_rules NO rastrea las
      cabeceras. Si cambias un #define de aquí y ejecutas `make`, hola.c no se
      recompila (su .obj es más nuevo que hola.c) y la ROM sigue con el texto
      viejo. Hay que hacer `make clean` y después `make` (README, paso 4).

    SE CONECTA CON
      Incluye:  nada.
      Lo usan:  src/hola.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES
      ninguna; constantes TEXTO_SALUDO, TEXTO_UNIDAD, TEXTO_PIE, FILA_SALUDO.

    → glosario: #define, #ifndef
---------------------------------------------------------------------------------*/
#ifndef TEXTOS_H                     /* Guarda de inclusión (unidad 00).                   */
#define TEXTOS_H

#define TEXTO_SALUDO "HELLO, SNES!"            /* El texto en pantalla va en inglés.      */
#define TEXTO_UNIDAD "UNIT 03: FROM .C TO .SFC" /* Segunda línea.                         */
#define TEXTO_PIE "BUILT WITH PVSNESLIB"        /* Tercera línea.                          */
#define FILA_SALUDO 10                          /* Fila (de 0 a 27) de la primera línea.   */

#endif /* TEXTOS_H */
