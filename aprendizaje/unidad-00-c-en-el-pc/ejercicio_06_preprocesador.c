/*---------------------------------------------------------------------------------
    ejercicio_06_preprocesador.c — Ejercicio 6 de la unidad 00: #define, macros,
    #ifdef y -D.

    PROPÓSITO
      Escribir las constantes y macros que usa main y decidir con #ifndef /
      #ifdef qué se compila según la línea de gcc. Las cinco definiciones de
      abajo están puestas con un valor provisional (0 o "?") para que el
      archivo compile desde el principio; cámbialas por las buenas. main ya
      está escrito: no lo toques; solo rellena los TODO 1 a 5.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf).
      Lo usan:  nadie: es un programa independiente de PC.
      Modelo:   src/sistema/pantallas.h (constantes hechas de otras, SEGUNDOS),
                src/sistema/tipos.h:33 y src/juego/mando.c (#ifdef).
      Compilar (las dos, sin avisos):
                gcc -std=c99 -Wall -Wextra -o ejercicio_06.exe ejercicio_06_preprocesador.c
                gcc -std=c99 -Wall -Wextra -DGUION_PRUEBA -DVIDAS_INICIALES=5 -o ejercicio_06d.exe ejercicio_06_preprocesador.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      ninguna: todo lo de este ejercicio son #define.

    → conceptos (README de la unidad 00): #define, macro, #ifdef, #ifndef, -D
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf.                                      */

/*
 * SALIDA ESPERADA sin -D (exacta):
 *   Pantallas del nivel: 8
 *   MAX(5, 1 + 2) * 2 = 10
 *   MAX(7, 2) = 7
 *   ENTRE(130, 128, 255) = 1
 *   ENTRE(300, 128, 255) = 0
 *   ENTRE(128, 128, 255) = 1
 *   Vidas iniciales: 3
 *   Mando: jugador
 *
 * SALIDA ESPERADA con -DGUION_PRUEBA -DVIDAS_INICIALES=5 (exacta): las seis
 * primeras líneas iguales y las dos últimas así:
 *   Vidas iniciales: 5
 *   Mando: guion
 */

#define ANCHO_PANTALLA 256           /* Píxeles de una pantalla.                          */
#define ANCHO_NIVEL 2048             /* Píxeles del nivel 1.                              */

/* TODO 1: PANTALLAS_NIVEL tiene que ser ANCHO_NIVEL dividido entre
           ANCHO_PANTALLA, escrito con esas dos constantes (no con un 8). Ponle
           paréntesis alrededor: es una expresión, no un número suelto. */
#define PANTALLAS_NIVEL 0

/* TODO 2: MAX(a, b) da el mayor de los dos. Usa el operador condicional
           «condición ? si_cierto : si_falso». Paréntesis en cada parámetro y
           alrededor de todo: si te sale MAX(5, 1 + 2) * 2 = 5, te faltan los
           de fuera. */
#define MAX(a, b) 0

/* TODO 3: ENTRE(x, lo, hi) da 1 si lo <= x <= hi y 0 si no. En C no se
           puede escribir «lo <= x <= hi»: son dos comparaciones unidas con &&. */
#define ENTRE(x, lo, hi) 0

/* TODO 4: VIDAS_INICIALES vale 3, salvo que se compile con -DVIDAS_INICIALES=N.
           Tal como está, compilar con -DVIDAS_INICIALES=5 da el aviso
           «"VIDAS_INICIALES" redefined» y gana el 0. Rodea el #define con
           #ifndef ... #endif para que solo se defina si nadie lo hizo antes. */
#define VIDAS_INICIALES 0

/* TODO 5: ORIGEN_MANDO es la cadena "guion" si está definido GUION_PRUEBA y
           "jugador" si no. Usa #ifdef / #else / #endif con un #define en
           cada rama, como hace src/juego/mando.c. */
#define ORIGEN_MANDO "?"

/*--- main: ya está escrito, no lo cambies -------------------------------------------*/
int main(void)
{
    printf("Pantallas del nivel: %d\n", PANTALLAS_NIVEL);
    printf("MAX(5, 1 + 2) * 2 = %d\n", MAX(5, 1 + 2) * 2);
    printf("MAX(7, 2) = %d\n", MAX(7, 2));
    printf("ENTRE(130, 128, 255) = %d\n", ENTRE(130, 128, 255));
    printf("ENTRE(300, 128, 255) = %d\n", ENTRE(300, 128, 255));
    printf("ENTRE(128, 128, 255) = %d\n", ENTRE(128, 128, 255));   /* El borde cuenta.   */
    printf("Vidas iniciales: %d\n", VIDAS_INICIALES);
    printf("Mando: %s\n", ORIGEN_MANDO);
    return 0;                        /* Terminó bien.                                     */
}
