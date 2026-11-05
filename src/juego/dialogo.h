/*---------------------------------------------------------------------------------
    dialogo.h — Las escenas de texto de la fase 1: páginas de dos líneas.

    PROPÓSITO
      Una escena de texto (Rex al entrar en el monasterio, Gomasuri-dono al caer) es una
      lista de PÁGINAS de dos líneas. Mientras dura, el mundo se congela (como en la pausa,
      pero sin media luz) y las dos líneas se escriben en las filas de aviso del HUD. Cada
      página se queda al menos DIALOGO_MIN frames (para que un A pulsado de antes no se la
      salte) y A la pasa; si nadie pulsa, pasa sola a los DIALOGO_MAX (la ROM que se juega
      sola no se queda parada). Lógica pura, sin snes.h (tests/prueba_dialogo.c).

    SE CONECTA CON
      Incluye:  sistema/tipos.h.
      Implementa: juego/dialogo.c.
      Lo usan:  pantallas/pantalla_fase1.c y tests/prueba_dialogo.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (campos de Dialogo)
      lineas   const char *const *  las líneas (2 por página)
      paginas  u8                   cuántas páginas
      pagina   u8                   la que se ve
      reloj    u8                   frames que lleva la página
      activo   u8                   1 mientras dura la escena
    → glosario: Máquina de estados
---------------------------------------------------------------------------------*/
#ifndef DIALOGO_H
#define DIALOGO_H

#include "sistema/tipos.h"

#define DIALOGO_MIN 24               /* Frames antes de que A pueda pasar la página.       */
#define DIALOGO_MAX 200              /* Frames tras los que la página pasa sola.           */

typedef struct
{
    const char *const *lineas;
    u8 paginas;
    u8 pagina;
    u8 reloj;
    u8 activo;
} Dialogo;

/* Sin escena. */
void dialogo_iniciar(Dialogo *d);

/* Empieza una escena de `paginas` páginas (2 * paginas líneas). */
void dialogo_empezar(Dialogo *d, const char *const *lineas, u8 paginas);

/* Un frame: devuelve 1 si hay que volver a escribir las líneas (página nueva o fin). */
u8 dialogo_actualizar(Dialogo *d, u8 avanzar_pulsado);

/* La línea k (0 o 1) de la página que se ve. */
const char *dialogo_linea(const Dialogo *d, u8 k);

#endif /* DIALOGO_H */
