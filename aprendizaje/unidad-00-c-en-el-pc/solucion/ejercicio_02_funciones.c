/*---------------------------------------------------------------------------------
    solucion/ejercicio_02_funciones.c — Solución del ejercicio 2 de la unidad 00.

    PROPÓSITO
      La solución que escribió el autor del proyecto (Etapa 0), con sus notas.
      Al pasar a la unidad 00 (2026-10-19) solo cambió el texto de los printf
      para que la salida coincida con la del enunciado. La lógica es la suya.

    SE CONECTA CON
      Incluye:  <stdio.h> (printf).
      Lo usan:  nadie: es un programa independiente de PC.
      Compilar: gcc -std=c99 -Wall -Wextra -o solucion_02.exe solucion/ejercicio_02_funciones.c

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      edad     int    0..10 (años del juego)    parámetro de la función y contador del for

    → conceptos (README de la unidad 00): función, parámetro, void
---------------------------------------------------------------------------------*/
#include <stdio.h>

// TODO 1: la funcion, definida ANTES de main().
void imprimirEtapaDeVida(int edad)
{
    if (edad <= 2)
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
// (El ";" después de la "}" de una función sobra: gcc lo acepta, pero el C
// estándar estricto no, y con -pedantic avisa. En el proyecto no se pone.)

int main(void)
{
    // TODO 2: el for llama a la funcion en vez de repetir el if/else.
    for (int edad = 0; edad <= 10; edad++)
    // Los argumentos de la funcion van entre parentesis, y se separan con comas si hay mas de uno.
    // En este caso solo hay uno: "edad". int al principio de la funcion indica que devuelve un entero,
    // pero como es void, no devuelve nada. El void está en la definición de la función
    // imprimirEtapaDeVida(), no en la llamada.
    {
        imprimirEtapaDeVida(edad);
    }

    return 0;
}
