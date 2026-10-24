/*---------------------------------------------------------------------------------
    ejercicio_07/marcador.c — Implementación del marcador (ejercicio 7 de la unidad 00).

    PROPÓSITO
      Escribir el «cómo» del marcador que promete marcador.h, con las tres
      formas de static: una variable de archivo (total), una función de
      archivo (comprobar_record) y una variable local que conserva su valor
      entre llamadas (veces). La variable global marcador_record ya está
      definida abajo. Solo rellena los TODO 1 a 4; main.c y marcador.h ya
      están escritos.

    SE CONECTA CON
      Incluye:  marcador.h.
      Lo usan:  ejercicio_07/main.c, a través de marcador.h.
      Modelo:   src/sistema/secuencia.c (define la variable que su .h declara
                extern), src/juego/tarou.c (funciones static internas).
      Compilar (desde la carpeta de la unidad, los dos .c a la vez):
                gcc -std=c99 -Wall -Wextra -o ejercicio_07.exe ejercicio_07/main.c ejercicio_07/marcador.c

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES
      marcador_record  u16  global, definida aquí     el récord (main.c lo ve por extern)
      total            u16  static de archivo         lo vas a declarar en el TODO 1
      veces            u8   static local de sumar     lo vas a declarar en el TODO 3

    → conceptos (README de la unidad 00): definición, static, extern
---------------------------------------------------------------------------------*/
#include "marcador.h"                /* Su propio .h: el compilador comprueba las firmas. */

/*
 * SALIDA ESPERADA del programa completo (exacta):
 *   Partida 1: total=0 record=0
 *   Suma 1: total=100 record=100
 *   Suma 2: total=350 record=350
 *   Partida 2: total=0 record=350
 *   Suma 3: total=50 record=350
 *   Suma 4: total=450 record=450
 *
 * Fíjate en la línea «Suma 3»: el número de sumas NO vuelve a 1 al empezar la
 * partida 2. Eso es lo que hace una static local: nadie de fuera puede
 * ponerla a cero.
 */

u16 marcador_record;                 /* DEFINICIÓN de la variable que marcador.h anuncia.
                                        Sin ella, el enlazador diría «undefined reference
                                        to `marcador_record'». main.c la pone a 0.         */

/* TODO 1: declara aquí una variable de archivo "total" (u16) que solo se vea
           en este .c (static). Después, en marcador_iniciar, ponla a 0. No
           cuentes con que una static empieza a 0: en la SNES no es así. */

/* TODO 2: escribe aquí una función static "comprobar_record" (void, sin
           parámetros) que copie total en marcador_record si total es mayor.
           Tiene que estar ANTES de marcador_sumar, que la va a llamar (o
           llevar su prototipo antes). Ojo: una función static que nadie llama
           da el aviso «defined but not used»; desaparece al hacer el TODO 3. */

void marcador_iniciar(void)
{
    /* TODO 1 (segunda parte): total a 0. El récord NO se toca. */
}

/* TODO 3: suma `puntos` a total, llama a comprobar_record y devuelve cuántas
           veces se ha llamado a marcador_sumar desde que empezó el programa.
           Para contarlo, declara dentro de la función:
               static u8 veces = 0;
           súmale 1 y devuélvela. */
u8 marcador_sumar(u16 puntos)
{
    (void)puntos;                    /* Calla «parámetro sin usar»: bórrala en el TODO 3. */
    return 0;
}

/* TODO 4: devuelve total. */
u16 marcador_total(void)
{
    return 0;
}
