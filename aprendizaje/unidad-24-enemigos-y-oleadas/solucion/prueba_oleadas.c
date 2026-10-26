/*---------------------------------------------------------------------------------
    prueba_oleadas.c — Pruebas de juego/oleadas.c (las zonas y el bloqueo de cámara).

    PROPÓSITO
      Fijar la máquina de estados: la cámara avanza hasta el punto de la zona, ahí se
      bloquea (y avisa UNA vez), vencer la oleada la libera y pasa a la zona siguiente, y
      tras la última zona ya no hay límite. Y que, juntos con camara_avanzar, la cámara
      se pare en el punto de la zona aunque el protagonista siga corriendo.

    SE CONECTA CON
      Incluye:  minitest.h, juego/oleadas.h, juego/camara.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_oleadas).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Zona, Oleada, Cámara
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/oleadas.h"
#include "juego/camara.h"

void pruebas_oleadas(void)
{
    Oleadas o;
    u16 camara, x;
    u8 avisos;

    oleadas_iniciar(&o);
    COMPROBAR(o.actual == 0 && o.bloqueada == 0);
    COMPROBAR(oleadas_limite(&o) == 256);            /* La primera zona está en la cámara 256. */
    COMPROBAR(oleadas_actualizar(&o, 100) == 0);     /* Lejos: nada.                       */
    COMPROBAR(oleadas_actualizar(&o, 256) == 1);     /* Llega: se bloquea y avisa.         */
    COMPROBAR(o.bloqueada == 1);
    COMPROBAR(oleadas_actualizar(&o, 256) == 0);     /* Ya bloqueada: no avisa otra vez.   */

    oleadas_vencer(&o);                              /* Oleada vencida: libre.             */
    COMPROBAR(o.bloqueada == 0 && o.actual == 1);
    COMPROBAR(oleadas_limite(&o) == 640);            /* Ahora, hasta la segunda.           */
    COMPROBAR(oleadas_actualizar(&o, 300) == 0);     /* La cámara sigue en 300 < 640.      */
    oleadas_vencer(&o);                              /* Sin estar bloqueada: no hace nada. */
    COMPROBAR(o.actual == 1);

    /* Las cinco zonas, y después ya no hay límite (ejercicio 24: antes eran cuatro). */
    oleadas_actualizar(&o, 640);
    oleadas_vencer(&o);
    oleadas_actualizar(&o, 1024);
    oleadas_vencer(&o);
    oleadas_actualizar(&o, 1536);
    oleadas_vencer(&o);
    oleadas_actualizar(&o, 1792);
    oleadas_vencer(&o);
    COMPROBAR(o.actual == OLEADAS_ZONAS && o.bloqueada == 0);
    COMPROBAR(oleadas_limite(&o) == 0xFFFF);
    COMPROBAR(oleadas_actualizar(&o, 1792) == 0);    /* Sin zonas que bloqueen.            */

    /* Con la cámara real: el protagonista corre y la cámara se para en el punto. */
    oleadas_iniciar(&o);
    camara = 0;
    avisos = 0;
    for (x = 100; x < 1000; x = (u16)(x + 2))        /* Siempre más lejos que la cámara.   */
    {
        camara = camara_avanzar(camara, x, oleadas_limite(&o));
        avisos = (u8)(avisos + oleadas_actualizar(&o, camara));
    }
    COMPROBAR(camara == 256);                        /* Se paró en el punto de la zona 0.  */
    COMPROBAR(avisos == 1);                          /* Y avisó una sola vez.              */
    oleadas_vencer(&o);                              /* Vencida: la cámara sigue...        */
    for (x = 1000; x < 1100; x = (u16)(x + 2))
    {
        camara = camara_avanzar(camara, x, oleadas_limite(&o));
    }
    COMPROBAR(camara > 256 && camara <= 640);       /* ...hasta la zona 1 como mucho.     */

    /*--- Ejercicio 24: una QUINTA zona al final del nivel (cámara 1792 = CAMARA_MAX) ------*/
    oleadas_iniciar(&o);
    for (x = 0; x < 4; x++)                          /* Pasa las cuatro primeras zonas.    */
    {
        oleadas_actualizar(&o, oleadas_limite(&o));  /* Llega al punto de la zona...       */
        oleadas_vencer(&o);                          /* ...y la vence.                     */
    }
    COMPROBAR(o.actual == 4 && o.bloqueada == 0);    /* Ahora toca la quinta.              */
    COMPROBAR(oleadas_limite(&o) == 1792);           /* Su punto es el final del nivel.    */
    camara = 1700;                                   /* La cámara viene de más atrás...    */
    for (x = 0; x < 200; x++)                        /* ...y Tarou corre hasta el borde.   */
    {
        camara = camara_avanzar(camara, 2000, oleadas_limite(&o));
    }
    COMPROBAR(camara == 1792);                       /* Llega exacta al punto.             */
    COMPROBAR(oleadas_actualizar(&o, camara) == 1 && o.bloqueada == 1);   /* Bloquea y avisa. */
    oleadas_vencer(&o);
    COMPROBAR(o.actual == OLEADAS_ZONAS && OLEADAS_ZONAS == 5);   /* Ya no quedan.         */
    COMPROBAR(oleadas_limite(&o) == 0xFFFF);
}
