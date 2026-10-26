/*---------------------------------------------------------------------------------
    prueba_carga.h — ROM de prueba de carga del beat 'em up (solo con -DPRUEBA_CARGA).

    PROPÓSITO
      Medir, antes de pedir el arte de los ninjas a Pixellab, cuántos enemigos
      humanos caben en pantalla con Tarou (docs/diseno-beat-em-up.md §9): las
      ranuras de sprite por línea, la VRAM de sprites, el tiempo del VBlank, la
      CPU de llenar la OAM y ordenar por profundidad, y cómo se ven las sombras
      con media suma de color. Recorre 10 configuraciones, 6 s cada una, sin
      tocar el mando; el HUD de depuración enseña la configuración y los máximos.

    SE CONECTA CON
      Implementa: pantallas/prueba_carga.c.
      Lo usan:  sistema/main.c (con -DPRUEBA_CARGA llama a prueba_carga_ejecutar
                en lugar de arrancar la secuencia de pantallas).

    HARDWARE QUE TOCA
      ver prueba_carga.c.

    VARIABLES: ninguna.
    → glosario: Prueba de carga, Ranura por línea, VBlank
---------------------------------------------------------------------------------*/
#ifndef PRUEBA_CARGA_H
#define PRUEBA_CARGA_H

/* Carga la escena de prueba y se queda en su bucle para siempre (no vuelve). */
void prueba_carga_ejecutar(void);

#endif /* PRUEBA_CARGA_H */
