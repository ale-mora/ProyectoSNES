/*---------------------------------------------------------------------------------
    escena_dialogo.h — Las dos tablas del motor de escenas para las escenas de diálogo
    de la fase 1: entrar (la imagen se apaga y se pixela) y salir (vuelve nítida).

    PROPÓSITO
      Cuando salta una escena (Lucinda, Rex, el jefe...), la acción se para y el juego
      "pasa a segundo plano": el brillo baja de 15 a 6 y el mosaico sube de 0 a 5 en 24
      frames (ESCENA_DIALOGO_ENTRA). Mientras se leen los cuadros de texto, la escena
      sigue con esos valores. Al terminar, la tabla de salida los devuelve a 15 y 0 en
      otros 24 frames (ESCENA_DIALOGO_SALE). El texto va en el fondo 2 (BG3), que el mosaico
      no toca (se prepara solo para los fondos 0 y 1), así que se lee nítido.
      Son datos para escena.c: cada RAMPA parte del valor que la pista tiene en ese
      momento (lo comprueba tests/prueba_dialogos.c).

    SE CONECTA CON
      Incluye:  escena/escena.h.
      Lo usan:  pantallas/pantalla_fase.c y tests/prueba_dialogos.c.

    HARDWARE QUE TOCA
      ninguno: son datos (efectos.c los lleva a la PPU).

    VARIABLES (constantes y tablas)
      ESCENA_DIALOGO_FRAMES      24   lo que dura cada tabla
      ESCENA_DIALOGO_BRILLO       6   el brillo mientras se lee (0..15)
      ESCENA_DIALOGO_MOSAICO      5   el mosaico mientras se lee (0..15)
      ESCENA_DIALOGO_ENTRA/SALE   EventoEscena[]  las tablas
    → glosario: Motor de escenas, Mosaico
---------------------------------------------------------------------------------*/
#ifndef ESCENA_DIALOGO_H
#define ESCENA_DIALOGO_H

#include "escena/escena.h"

#define ESCENA_DIALOGO_FRAMES 24
#define ESCENA_DIALOGO_BRILLO 6
#define ESCENA_DIALOGO_MOSAICO 5
#define ESCENA_DIALOGO_EVENTOS 5

extern const EventoEscena ESCENA_DIALOGO_ENTRA[ESCENA_DIALOGO_EVENTOS];
extern const EventoEscena ESCENA_DIALOGO_SALE[ESCENA_DIALOGO_EVENTOS];

#endif /* ESCENA_DIALOGO_H */
