/*---------------------------------------------------------------------------------
    prueba_camara.c — Pruebas de juego/camara.c y juego/nivel.c.

    PROPÓSITO
      Fijar con ejemplos la regla de la cámara (docs/diseno-demo-jugable.md §6):
      camara = x_pix + 48 - 128 (48 = mitad de la celda de 96 px), recortada a
      0..ANCHO_NIVEL - ANCHO_PANTALLA.
      Y la cuenta del mapa-anillo de la VRAM (nivel_columna_vram).

    SE CONECTA CON
      Incluye:  minitest.h, juego/camara.h, juego/nivel.h.
      Lo usan:  tests/pruebas.c (llama a pruebas_camara).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna.
    → glosario: Cámara, Suelo, Nivel
---------------------------------------------------------------------------------*/
#include "minitest.h"
#include "juego/camara.h"
#include "juego/nivel.h"

void pruebas_camara(void)
{
    /* Mientras Tarou está cerca del borde izquierdo, la cámara no se mueve. */
    COMPROBAR(camara_para(0) == 0);
    COMPROBAR(camara_para(80) == 0);              /* 80 + 48 - 128 = 0: justo el límite.  */
    COMPROBAR(camara_para(81) == 1);              /* Un píxel más y empieza a seguirlo.   */
    COMPROBAR(camara_para(200) == 120);           /* 200 + 48 - 128.                      */

    /* En el borde derecho se detiene en ANCHO_NIVEL - ANCHO_PANTALLA = 1792. */
    COMPROBAR(camara_para(1872) == 1792);         /* 1872 + 48 - 128 = 1792: el límite.   */
    COMPROBAR(camara_para(1952) == 1792);         /* Más allá (el tope de Tarou), sigue.  */

    /* Otros anchos de nivel (camara_para_ancho): el tope es ancho - 256. */
    COMPROBAR(camara_para_ancho(1872, 2048) == 1792); /* El de siempre, por la nueva.  */
    COMPROBAR(camara_para_ancho(500, 1024) == 420);   /* 500 + 48 - 128, sin tope.      */
    COMPROBAR(camara_para_ancho(848, 1024) == 768);   /* 848 + 48 - 128 = 768: el tope. */
    COMPROBAR(camara_para_ancho(900, 1024) == 768);   /* Más allá, sigue en 1024 - 256. */
    COMPROBAR(camara_para_ancho(10, 1024) == 0);      /* El borde izquierdo no cambia.  */
    COMPROBAR(camara_para_ancho(200, 256) == 0);      /* Nivel de una pantalla: quieta. */
    COMPROBAR(camara_para_ancho(81, 512) == 1);       /* Un nivel de 512 (el anillo)... */
    COMPROBAR(camara_para_ancho(400, 512) == 256);    /* ...para en 256.                */

    /* Constantes del escenario. */
    COMPROBAR(SUELO_Y == 176);                    /* Suelo base: tres filas de celdas.    */
    COMPROBAR(ANCHO_NIVEL == 2048);               /* El tutorial: 8 pantallas.            */
    COMPROBAR(COLUMNAS_NIVEL == 256);
    COMPROBAR(ANCHO_PANTALLA == 256);
    COMPROBAR(ANCHO_CELDA == 96);                 /* Celda de 96x64: el perro de Pixellab */
    COMPROBAR(ALTO_CELDA == 64);                  /* mide hasta 81 px de ancho al correr. */

    /* Columnas del nivel → mapa-anillo de 64x32 de la VRAM (fondo_nivel.c). */
    COMPROBAR(nivel_columna_vram(0) == 0);        /* Primera página, primera columna.     */
    COMPROBAR(nivel_columna_vram(31) == 31);      /* Última de la primera página.         */
    COMPROBAR(nivel_columna_vram(32) == 0x400);   /* Segunda página (0x400 palabras más). */
    COMPROBAR(nivel_columna_vram(63) == 0x41F);
    COMPROBAR(nivel_columna_vram(64) == 0);       /* El anillo da la vuelta cada 64.      */
    COMPROBAR(nivel_columna_vram(100) == 0x400 + 4); /* 100 = 64 + 32 + 4.                */
    COMPROBAR(nivel_columna_vram(255) == 0x41F);  /* La última columna del nivel.         */
}
