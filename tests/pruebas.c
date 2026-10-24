/*---------------------------------------------------------------------------------
    pruebas.c — Programa de PC que ejecuta todas las suites de pruebas.

    PROPÓSITO
      Define los contadores de minitest.h, llama a cada suite (una por archivo
      prueba_*.c), imprime el resumen y devuelve 0 solo si no hubo fallos (así
      `make -C tests && tests/pruebas.exe` corta en rojo).

    SE CONECTA CON
      Incluye:  minitest.h.
      Lo usan:  tests/Makefile (lo compila junto con prueba_*.c y la lógica pura
                de ../src/juego/).

    HARDWARE QUE TOCA
      ninguno (solo PC).

    VARIABLES (globales)
      pruebas_total     u16  comprobaciones ejecutadas (ver minitest.h)
      pruebas_fallidas  u16  comprobaciones fallidas

    → glosario: u8/u16
---------------------------------------------------------------------------------*/
#include "minitest.h"

u16 pruebas_total = 0;
u16 pruebas_fallidas = 0;

/* Cada suite vive en su propio archivo y se declara aquí (no hace falta header). */
void pruebas_puntofijo(void);
void pruebas_camara(void);
void pruebas_animacion(void);
void pruebas_tarou(void);
void pruebas_metasprite(void);
void pruebas_colision(void);
void pruebas_partida(void);
void pruebas_recorrido(void);
void pruebas_escena(void);
void pruebas_profundidad(void);
void pruebas_subida(void);
void pruebas_p16(void);
void pruebas_mundo(void);
void pruebas_oleadas(void);

int main(void)
{
    pruebas_puntofijo();
    pruebas_camara();
    pruebas_animacion();
    pruebas_tarou();
    pruebas_metasprite();
    pruebas_colision();
    pruebas_partida();
    pruebas_recorrido();
    pruebas_escena();
    pruebas_profundidad();
    pruebas_subida();
    pruebas_p16();
    pruebas_mundo();
    pruebas_oleadas();

    printf("%u pruebas, %u fallos\n", (unsigned)pruebas_total, (unsigned)pruebas_fallidas);
    return pruebas_fallidas != 0;
}
