/*---------------------------------------------------------------------------------
    prueba_marcador.c — Pruebas de juego/marcador.c.

    PROPÓSITO
      Fijar la cuenta del arcade (docs/diseno-beat-em-up.md §5.5 y decisión 12): los
      puntos suman con acarreo sin `long` (resto y miles); la vida extra sale a
      50 000 y a 150 000, una sola vez cada una; perder la última vida acaba la
      partida; un continue vuelve con las vidas de inicio y se gasta.

    SE CONECTA CON
      Incluye:  minitest.h, juego/marcador.h, juego/cifras.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_marcador).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Vida extra, Continue, Puntuación
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/marcador.h"
#include "juego/cifras.h"

void pruebas_marcador(void)
{
    Marcador m;
    char txt[12];
    u8 dadas;
    u16 i;

    marcador_iniciar(&m);
    COMPROBAR(m.resto == 0 && m.miles == 0);
    COMPROBAR(m.vidas == VIDAS_INICIALES && m.continues == CONTINUES_INICIALES);

    /* Suma simple y acarreo al pasar de 10 000. */
    marcador_sumar(&m, 9990);
    COMPROBAR(m.resto == 9990 && m.miles == 0);
    marcador_sumar(&m, 20);
    COMPROBAR(m.resto == 10 && m.miles == 1);

    /* Una suma de 9999 justa en el límite no hace acarreo de más. */
    marcador_iniciar(&m);
    marcador_sumar(&m, 9999);
    COMPROBAR(m.resto == 9999 && m.miles == 0);
    marcador_sumar(&m, 1);
    COMPROBAR(m.resto == 0 && m.miles == 1);

    /* Vida extra a 50 000 (5 miles): exactamente ahí, no antes. */
    marcador_iniciar(&m);
    dadas = 0;
    for (i = 0; i < 4; i++)
        dadas = (u8)(dadas + marcador_sumar(&m, 9999));
    COMPROBAR(m.miles == 3 && m.extra_50k == 0 && dadas == 0);
    dadas = marcador_sumar(&m, 9999);
    COMPROBAR(dadas == 0 && m.miles == 4);
    dadas = marcador_sumar(&m, 9999);
    COMPROBAR(dadas == 1 && m.extra_50k == 1 && m.vidas == VIDAS_INICIALES + 1);

    /* No se repite: volver a sumar no da otra vida de 50 000. */
    dadas = marcador_sumar(&m, 9999);
    COMPROBAR(dadas == 0 && m.vidas == VIDAS_INICIALES + 1);

    /* Vida extra a 150 000 (15 miles), también una sola vez. */
    while (m.miles < MARCADOR_EXTRA_150K_MILES - 1)
        marcador_sumar(&m, 9999);
    COMPROBAR(m.extra_150k == 0);
    dadas = 0;
    while (m.miles < MARCADOR_EXTRA_150K_MILES + 1)
        dadas = (u8)(dadas + marcador_sumar(&m, 9999));
    COMPROBAR(m.extra_150k == 1 && m.vidas == VIDAS_INICIALES + 2);
    COMPROBAR(dadas == 1);

    /* Saltar de golpe por encima de las dos: da las dos a la vez. */
    marcador_iniciar(&m);
    m.miles = 14;
    m.resto = 9000;
    dadas = marcador_sumar(&m, 9000);
    COMPROBAR(dadas == 2 && m.vidas == VIDAS_INICIALES + 2);
    COMPROBAR(m.miles == 15 && m.resto == 8000);

    /* Texto de los puntos: sin ceros delante; con miles, el resto va a 4 dígitos. */
    marcador_iniciar(&m);
    marcador_texto(&m, txt);
    COMPROBAR(txt[0] == '0' && txt[1] == 0);
    marcador_sumar(&m, 5);
    marcador_texto(&m, txt);
    COMPROBAR(txt[0] == '5' && txt[1] == 0);
    marcador_sumar(&m, 1229);
    marcador_texto(&m, txt);
    COMPROBAR(txt[0] == '1' && txt[1] == '2' && txt[2] == '3' && txt[3] == '4' && txt[4] == 0);
    marcador_iniciar(&m);
    marcador_sumar(&m, 9999);
    marcador_texto(&m, txt);
    COMPROBAR(txt[0] == '9' && txt[3] == '9' && txt[4] == 0);
    marcador_iniciar(&m);
    m.miles = 1;
    m.resto = 5;
    marcador_texto(&m, txt);
    COMPROBAR(txt[0] == '1' && txt[1] == '0' && txt[2] == '0' && txt[3] == '0' && txt[4] == '5' && txt[5] == 0);
    m.miles = 15;
    m.resto = 8000;
    marcador_texto(&m, txt);
    COMPROBAR(txt[0] == '1' && txt[1] == '5' && txt[2] == '8' && txt[3] == '0' && txt[4] == '0' && txt[5] == '0' && txt[6] == 0);

    /* Perder vidas: la última acaba la partida. */
    marcador_iniciar(&m);
    COMPROBAR(marcador_perder_vida(&m) == 0 && m.vidas == VIDAS_INICIALES - 1);
    COMPROBAR(marcador_perder_vida(&m) == 0 && m.vidas == VIDAS_INICIALES - 2);
    COMPROBAR(marcador_perder_vida(&m) == 1 && m.vidas == 0);
    COMPROBAR(marcador_perder_vida(&m) == 1 && m.vidas == 0);   /* No baja de 0.           */

    /* Continue: vuelve con las vidas de inicio y gasta uno. */
    COMPROBAR(marcador_continuar(&m) == 1);
    COMPROBAR(m.vidas == VIDAS_INICIALES && m.continues == CONTINUES_INICIALES - 1);
    while (m.continues > 0)
        marcador_continuar(&m);
    COMPROBAR(m.continues == 0);
    COMPROBAR(marcador_continuar(&m) == 0);
}
