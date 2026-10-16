#include <stdio.h>

int main(void)
{
    /*
     * EJERCICIO 1: Logica de programacion (variables, condicionales, bucles)
     *
     * Objetivo: usando un bucle for, una variable entera, y condicionales
     * if / else if / else, simular como cambia la etapa de vida de Tarouh
     * (cachorro -> adulto -> viejo) a medida que pasan los "anios" del juego.
     *
     * Reglas:
     *   - edad < 3         -> "cachorro"
     *   - edad >= 3 y < 8   -> "adulto"
     *   - edad >= 8         -> "viejo"
     *
     * Salida esperada (aproximada), una linea por cada anio de 0 a 10:
     *   Anio 0: Tarouh es cachorro
     *   Anio 1: Tarouh es cachorro
     *   Anio 2: Tarouh es cachorro
     *   Anio 3: Tarouh es adulto
     *   ...
     *   Anio 8: Tarouh es viejo
     *   ...
     *   Anio 10: Tarouh es viejo
     *
     * Completa el codigo donde dice TODO. No hace falta que sea perfecto,
     * el objetivo es que compile y que el resultado tenga sentido.
     */

    // TODO 1 (resuelto como ejemplo): declaramos una variable entera
    // llamada "edad". Todavia no tiene un valor definido -- eso lo hace
    // la primera parte del "for" de abajo.
    int edad;

    // TODO 2 (resuelto como ejemplo): bucle for que hace que "edad" vaya
    // de 0 a 10 (incluyendo el 10), incrementando de a 1 en cada vuelta.
    // Nota la diferencia con el demo: aca "edad" ya fue declarada arriba,
    // asi que en el for solo la inicializamos (edad = 0), no volvemos a
    // escribir "int" adelante.
    for (edad = 0; edad <= 10; edad++)
    {
        // TODO 3: ESTA PARTE ES TUYA. Usa if / else if / else para
        //         imprimir una linea como "Anio 4: Tarouh es adulto"
        //         segun las reglas de arriba. Usa printf con %d para
        //         el numero de "edad". Mira el demo (demo_01_logica.c)
        //         si necesitas recordar la sintaxis de printf y de if.
        if (edad <= 2)
        // Error: El "=" es de asignación mientras que el "==" es de equivalencia.
        {
            printf("Tarouh es Cachorro y tiene %d anio. \n", edad);
        }
        else if (edad < 8)
        {
            printf("Tarouh es Adulto y tiene %d anios. \n", edad);
        }
        else
        {
            printf("Tarouh es Viejito y tiene %d anios. \n", edad);
        }
    };
    return 0;
}
