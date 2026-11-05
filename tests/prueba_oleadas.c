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
#include "enemigos/ninja.h"

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

    /* Las cuatro zonas, y después ya no hay límite. */
    oleadas_actualizar(&o, 640);
    oleadas_vencer(&o);
    oleadas_actualizar(&o, 1024);
    oleadas_vencer(&o);
    oleadas_actualizar(&o, 1536);
    oleadas_vencer(&o);
    COMPROBAR(o.actual == OLEADAS_ZONAS && o.bloqueada == 0);
    COMPROBAR(oleadas_limite(&o) == CAMARA_MAX);   /* Sin zonas: el final de la aldea. */
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

    /* Un punto de zona más allá de donde llega la cámara (CAMARA_MAX = 1792) se bloquea en el
       final del nivel; antes, con 1800, la cámara se quedaba en 1792 y la zona no salía nunca. */
    COMPROBAR(oleadas_alcanzable(1800) == CAMARA_MAX);
    COMPROBAR(oleadas_alcanzable(0xFFFF) == CAMARA_MAX);
    COMPROBAR(oleadas_alcanzable(1536) == 1536);     /* Los que caben no cambian.          */
    COMPROBAR(oleadas_alcanzable(CAMARA_MAX) == CAMARA_MAX);

    /* Y todas las zonas de la tabla se bloquean de verdad: la cámara, persiguiendo a un
       protagonista que corre hacia el final, llega a cada una. */
    oleadas_iniciar(&o);
    camara = 0;
    avisos = 0;
    for (x = 0; x < 6000; x++)
    {
        camara = camara_avanzar(camara, 2047, oleadas_limite(&o));
        if (oleadas_actualizar(&o, camara))
        {
            avisos++;
            oleadas_vencer(&o);                      /* Vencida al momento: a la siguiente. */
        }
    }
    COMPROBAR(avisos == OLEADAS_ZONAS && o.actual == OLEADAS_ZONAS);
    COMPROBAR(oleadas_terminadas(&o));

    /* La aldea, como antes: N; N+M; N+M+G; N+M+G. */
    oleadas_iniciar(&o);
    COMPROBAR(o.escenario == ESCENARIO_ALDEA && oleadas_ancho(ESCENARIO_ALDEA) == 2048);
    COMPROBAR(oleadas_cuantos(&o) == 1 && oleadas_tipos(&o)[0] == NINJA_NORMAL);
    o.actual = 2;
    COMPROBAR(oleadas_cuantos(&o) == 3 && oleadas_tipos(&o)[2] == NINJA_GRANDE);

    /* Los otros escenarios (1024 px): cada zona se alcanza, ninguna pasa del final
       (1024 - 256 = 768), al terminar la cámara llega al final y no más. */
    {
        u8 e;
        for (e = ESCENARIO_BOSQUE; e < ESCENARIOS; e++)
        {
            oleadas_iniciar_escenario(&o, e);
            COMPROBAR(oleadas_ancho(e) == 1024);
            COMPROBAR(!oleadas_terminadas(&o));
            camara = 0;
            avisos = 0;
            for (x = 0; x < 3000; x++)
            {
                camara = camara_avanzar(camara, 2047, oleadas_limite(&o));
                if (camara > 768)
                {
                    avisos = 100;                    /* Se pasó del final: falla abajo.     */
                }
                if (oleadas_actualizar(&o, camara))
                {
                    if (oleadas_cuantos(&o) >= 1 && oleadas_cuantos(&o) <= OLEADA_TIPOS)
                    {
                        avisos++;                    /* Una oleada con 1..3 enemigos.       */
                    }
                    oleadas_vencer(&o);
                }
            }
            COMPROBAR(avisos == o.zonas && oleadas_terminadas(&o));
            COMPROBAR(camara == 768);
        }
    }

    /* El dojo acaba con el jefe solo. */
    oleadas_iniciar_escenario(&o, ESCENARIO_DOJO);
    o.actual = 1;
    COMPROBAR(oleadas_cuantos(&o) == 1 && oleadas_tipos(&o)[0] == NINJA_JEFE);
    /* El bosque abre con un ninja rápido. */
    oleadas_iniciar_escenario(&o, ESCENARIO_BOSQUE);
    COMPROBAR(oleadas_tipos(&o)[0] == NINJA_RAPIDO);
}
