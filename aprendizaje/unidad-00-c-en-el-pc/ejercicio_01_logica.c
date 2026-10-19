/*---------------------------------------------------------------------------------
    ejercicio_01_logica.c — Ejercicio 1 de la unidad 00: variables, if y for.

    PROPÓSITO
      Practicar el vocabulario mínimo de C: una variable entera, un bucle for
      que la hace avanzar y una cadena if / else if / else que decide qué
      imprimir. Los TODO 1 y 2 están resueltos como ejemplo; el TODO 3 es tuyo.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf).
      Lo usan:  nadie: es un programa independiente de PC.
      Compilar: gcc -std=c99 -Wall -Wextra -o ejercicio_01.exe ejercicio_01_logica.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC, no en la SNES).

    VARIABLES
      edad     int    0..10 (años del juego)    la etapa de vida depende de ella

    → conceptos (README de la unidad 00): variable, if, for
---------------------------------------------------------------------------------*/
#include <stdio.h>                   /* Trae printf, que escribe texto en la consola.     */

int main(void)                       /* Todo programa de C empieza a ejecutarse en main.  */
{
    /*
     * EJERCICIO 1: lógica de programación (variables, condicionales, bucles)
     *
     * Objetivo: usando un bucle for, una variable entera y condicionales
     * if / else if / else, simular cómo cambia la etapa de vida de Tarou
     * (cachorro -> adulto -> viejo) a medida que pasan los años del juego.
     *
     * Reglas:
     *   - edad < 3           -> "cachorro"
     *   - edad >= 3 y < 8    -> "adulto"
     *   - edad >= 8          -> "viejo"
     *
     * SALIDA ESPERADA (exacta, una línea por año de 0 a 10):
     *   Anio 0: Tarou es cachorro
     *   Anio 1: Tarou es cachorro
     *   Anio 2: Tarou es cachorro
     *   Anio 3: Tarou es adulto
     *   Anio 4: Tarou es adulto
     *   Anio 5: Tarou es adulto
     *   Anio 6: Tarou es adulto
     *   Anio 7: Tarou es adulto
     *   Anio 8: Tarou es viejo
     *   Anio 9: Tarou es viejo
     *   Anio 10: Tarou es viejo
     *
     * (Las cadenas de printf van sin acentos ni eñes a propósito: la consola
     * de Windows no siempre los muestra bien y la salida dejaría de coincidir.)
     */

    /* TODO 1 (resuelto como ejemplo): declaramos una variable entera llamada
       "edad". Todavía no tiene un valor: se lo da la primera parte del for. */
    int edad;

    /* TODO 2 (resuelto como ejemplo): un for que hace que "edad" vaya de 0 a
       10 (incluido el 10), sumando 1 en cada vuelta. "edad" ya está declarada
       arriba, así que en el for solo se le da su valor inicial (edad = 0),
       sin volver a escribir "int" delante. */
    for (edad = 0; edad <= 10; edad++)
    {
        /* TODO 3: ESTA PARTE ES TUYA. Usa if / else if / else para imprimir
           una línea como "Anio 4: Tarou es adulto" según las reglas de arriba.
           Usa printf con %d para el número de "edad". Mira demo_01_logica.c
           si necesitas recordar la sintaxis de printf y de if.
           Cuidado: "=" asigna un valor, "==" compara dos valores. */
    }

    return 0;                        /* 0 le dice al sistema operativo "terminó bien".    */
}
