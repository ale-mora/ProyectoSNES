/*---------------------------------------------------------------------------------
    marcador.c — Puntuación, vidas y continues (ver marcador.h).

    PROPÓSITO
      Sumar sin `long`: el resto se suma y, si pasa de 9999, se resta 10 000 y se
      cuenta un «mil» más. Como cada suma es menor de 10 000, basta un acarreo.
      Las vidas extra se dan cuando `miles` llega a 5 y a 15, una sola vez cada
      una (la marca `extra_50k` / `extra_150k` lo recuerda).

    SE CONECTA CON
      Incluye:  juego/marcador.h, juego/cifras.h (VIDAS_INICIALES, CONTINUES_INICIALES).
      Lo usan:  tests/prueba_marcador.c.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna propia.
    → glosario: Vida extra, Continue
---------------------------------------------------------------------------------*/
#include "juego/marcador.h"
#include "juego/cifras.h"

void marcador_iniciar(Marcador *m)
{
    m->resto = 0;
    m->miles = 0;
    m->vidas = VIDAS_INICIALES;
    m->continues = CONTINUES_INICIALES;
    m->extra_50k = 0;
    m->extra_150k = 0;
}

u8 marcador_sumar(Marcador *m, u16 puntos)
{
    u8 dadas;                        /* Vidas extra que da esta suma.                      */

    dadas = 0;
    m->resto = m->resto + puntos;
    if (m->resto >= 10000)           /* Acarreo: se cuenta un mil más.                     */
    {
        m->resto = m->resto - 10000;
        m->miles++;
    }

    if (!m->extra_50k && m->miles >= MARCADOR_EXTRA_50K_MILES)
    {
        m->extra_50k = 1;
        m->vidas++;
        dadas++;
    }
    if (!m->extra_150k && m->miles >= MARCADOR_EXTRA_150K_MILES)
    {
        m->extra_150k = 1;
        m->vidas++;
        dadas++;
    }
    return dadas;
}

/* Potencias de diez de 10 000 a 1, para restar en vez de dividir. */
static const u16 POTENCIAS[5] = {10000, 1000, 100, 10, 1};

/* Escribe `v` en `s` con al menos `minimo` dígitos (0 y 1 cuentan como 1); devuelve
   cuántos escribió. No termina la cadena: lo hace quien llama. */
static u8 escribir(char *s, u16 v, u8 minimo)
{
    u8 n;                            /* Dígitos escritos hasta ahora.                       */
    u8 empezado;                     /* 1 cuando ya salió un dígito distinto de cero.       */
    u8 k;                            /* Potencia actual (0 = las diez mil).                 */
    u8 d;                            /* Dígito de esta potencia.                            */
    u8 forzar;                       /* Desde qué potencia hay que escribir siempre.        */

    n = 0;
    empezado = 0;
    forzar = (u8)(5 - (minimo > 1 ? minimo : 1));  /* 5 - 1 = 4: la última siempre sale.    */
    for (k = 0; k < 5; k++)
    {
        d = 0;
        while (v >= POTENCIAS[k])    /* Cuántas veces cabe la potencia.                     */
        {
            v = v - POTENCIAS[k];
            d++;
        }
        if (d > 0 || empezado || k >= forzar)
        {
            s[n] = (char)('0' + d);
            n++;
            empezado = 1;
        }
    }
    return n;
}

void marcador_texto(const Marcador *m, char *salida)
{
    u8 n;

    if (m->miles > 0)
    {
        n = escribir(salida, m->miles, 1);
        n = (u8)(n + escribir(salida + n, m->resto, 4));   /* Con 4 dígitos fijos.         */
    }
    else
    {
        n = escribir(salida, m->resto, 1);
    }
    salida[n] = 0;
}

u8 marcador_perder_vida(Marcador *m)
{
    if (m->vidas > 0)
        m->vidas--;
    return m->vidas == 0;            /* Sin vidas: se acabó la partida.                    */
}

u8 marcador_continuar(Marcador *m)
{
    if (m->continues == 0)
        return 0;
    m->continues--;
    m->vidas = VIDAS_INICIALES;      /* Vuelve con las vidas de inicio.                    */
    return 1;
}
