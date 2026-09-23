/*---------------------------------------------------------------------------------
    comun.c — Utilidades compartidas por las pantallas.

    Regla de PVSnesLib que se nota aquí: las variables se declaran al principio
    de cada función (C clásico) y preferimos variables globales a locales, porque
    el compilador de la SNES (tcc816) genera código más rápido con ellas.
---------------------------------------------------------------------------------*/
#include "comun.h"
#include "pantallas.h"   /* Para FRAMES_FADE si alguien lo necesita; y por consistencia. */

/*--- Variables de trabajo (globales al archivo) ---------------------------------*/
static char texto_parcial[33];  /* Búfer para el trozo de texto que se muestra en la
                                   máquina de escribir: 32 letras (ancho de pantalla)
                                   + 1 para el 0 final. "static" = solo se ve en este
                                   archivo. → glosario: static */
static u8 i;                    /* Índice para bucles. */
static u8 largo;                /* Largo del texto que estamos procesando. */
static u8 columna;              /* Columna (x) donde empieza un texto centrado. */

/*--- longitud -------------------------------------------------------------------
    En C una cadena es una fila de letras terminada en un byte 0. Contamos letras
    hasta encontrar ese 0. (Existe strlen en la librería, pero escribirla enseña
    cómo funcionan las cadenas.) */
u8 longitud(char *texto)
{
    u8 n = 0;                    /* Contador de letras, empieza en cero.                   */
    while (texto[n] != 0)        /* Mientras la letra en la posición n NO sea el 0 final...*/
    {
        n++;                     /* ...avanzamos una posición (n = n + 1).                 */
    }
    return n;                    /* n es ahora la cantidad de letras.                      */
}

/*--- limpiar_texto --------------------------------------------------------------
    La capa de texto tiene 32 columnas x 28 filas (256x224 píxeles / 8). Escribir
    32 espacios en cada fila la deja en blanco. La librería copia el resultado a
    la VRAM en el siguiente VBlank. → glosario: Consola de texto, VBlank */
void limpiar_texto(void)
{
    for (i = 0; i < 28; i++)     /* i recorre las filas 0, 1, 2 ... 27.                    */
    {
        consoleDrawText(0, i, "                                ");  /* 32 espacios.       */
    }
}

/*--- texto_centrado -------------------------------------------------------------
    Centrar = empezar en la columna (32 - largo) / 2. Ejemplo: "PRESS START" tiene
    11 letras → (32 - 11) / 2 = 10 (división entera, se descarta el resto). */
void texto_centrado(u8 fila, char *texto)
{
    largo = longitud(texto);             /* Cuántas letras tiene.                          */
    columna = (32 - largo) / 2;          /* Columna inicial para que quede centrado.       */
    consoleDrawText(columna, fila, texto);  /* PVSnesLib: escribe en la celda (x, y).      */
}

/*--- escribir_parcial -----------------------------------------------------------
    Efecto máquina de escribir para textos de varias líneas. Se llama una vez por
    línea, pasándole cuántas letras "hay disponibles" para mostrar:
      restante = escribir_parcial(10, "LINEA UNO", letras);
      restante = escribir_parcial(12, "LINEA DOS", restante);
    Si las letras alcanzan para toda la línea, se escribe completa y se devuelven
    las que sobran; si no, se escribe el trozo y se devuelve 0 (las líneas de
    abajo no muestran nada todavía). La columna se calcula con el largo TOTAL
    para que el texto no "baile" mientras crece. */
u16 escribir_parcial(u8 fila, char *texto, u16 letras)
{
    largo = longitud(texto);             /* Largo de la línea completa.                    */
    columna = (32 - largo) / 2;          /* Columna fija para centrar la línea completa.   */

    if (letras >= largo)                 /* ¿Hay letras suficientes para toda la línea?    */
    {
        consoleDrawText(columna, fila, texto);   /* Sí: la escribimos entera...            */
        return letras - largo;                   /* ...y devolvemos las que sobran.        */
    }

    for (i = 0; i < letras; i++)         /* No: copiamos solo las primeras `letras` letras */
    {
        texto_parcial[i] = texto[i];     /* ...una por una al búfer...                     */
    }
    texto_parcial[letras] = 0;           /* ...y cerramos la cadena con el 0 final.        */
    consoleDrawText(columna, fila, texto_parcial);  /* Mostramos el trozo.                 */
    return 0;                            /* No sobró nada para las líneas siguientes.      */
}

/*--- fade_in / fade_out ---------------------------------------------------------
    setFadeEffectEx anima el brillo de la PPU (0 = negro, 15 = normal) y BLOQUEA
    el programa hasta terminar; por eso las pantallas las llaman solo al entrar y
    al salir, nunca en medio de una animación. El segundo parámetro son frames
    según la documentación del SDK; la duración exacta se ajusta a ojo en Mesen.
    → glosario: Fade / brillo */
void fade_in(u8 frames)
{
    setFadeEffectEx(FADE_IN, frames);    /* De negro a brillo normal.                      */
}

void fade_out(u8 frames)
{
    setFadeEffectEx(FADE_OUT, frames);   /* De brillo normal a negro.                      */
}
