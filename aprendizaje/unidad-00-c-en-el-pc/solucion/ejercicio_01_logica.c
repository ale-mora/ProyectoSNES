/*---------------------------------------------------------------------------------
    solucion/ejercicio_01_logica.c — Solución del ejercicio 1 de la unidad 00.

    PROPÓSITO
      La solución que escribió el autor del proyecto (Etapa 0, 2026-09-22), con
      sus notas. Al pasar a la unidad 00 (2026-10-19) solo cambió el texto de
      los printf, para que la salida coincida exactamente con la del enunciado
      (antes imprimía "Tarou es Cachorro y tiene 0 anio."). La lógica es la suya.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf).
      Lo usan:  nadie: es un programa independiente de PC.
      Compilar: gcc -std=c99 -Wall -Wextra -o solucion_01.exe solucion/ejercicio_01_logica.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      edad     int    0..10 (años del juego)    la etapa de vida depende de ella

    → conceptos (README de la unidad 00): variable, if, for
---------------------------------------------------------------------------------*/
#include <stdio.h>

int main(void)
{
    /*
     * EJERCICIO 1: Logica de programacion (variables, condicionales, bucles)
     *
     * Reglas:
     *   - edad < 3         -> "cachorro"
     *   - edad >= 3 y < 8   -> "adulto"
     *   - edad >= 8         -> "viejo"
     *
     * SALIDA ESPERADA (exacta): la del enunciado, ejercicio_01_logica.c.
     */

    // TODO 1 (resuelto como ejemplo): declaramos una variable entera
    // llamada "edad". Todavia no tiene un valor definido -- eso lo hace
    // la primera parte del "for" de abajo.
    int edad;

    // TODO 2 (resuelto como ejemplo): bucle for que hace que "edad" vaya
    // de 0 a 10 (incluyendo el 10), incrementando de a 1 en cada vuelta.
    for (edad = 0; edad <= 10; edad++)
    {
        // TODO 3: la parte del autor.
        if (edad <= 2)
        // Error: El "=" es de asignación mientras que el "==" es de equivalencia.
        // (Con enteros, "edad <= 2" es lo mismo que la regla "edad < 3".)
        {
            printf("Anio %d: Tarou es cachorro\n", edad);
        }
        else if (edad < 8)
        {
            printf("Anio %d: Tarou es adulto\n", edad);
        }
        else
        {
            printf("Anio %d: Tarou es viejo\n", edad);
        }
    };
    return 0;
}
