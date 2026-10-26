/*---------------------------------------------------------------------------------
    pantalla_fase.h — La pantalla de la fase 1 del beat 'em up (solo con -DFASE1).

    PROPÓSITO
      Une el director de la fase (juego/fase.c: relevo, apoyos, oleadas, enemigos y marcador)
      con el hardware: el fondo que se desplaza, los sprites, el HUD de texto, las escenas
      de diálogo con el motor de escenas y las transiciones entre las cuatro zonas. Se queda
      en su propio bucle (no vuelve), como la prueba de carga y el prototipo.

    SE CONECTA CON
      Implementa: pantallas/pantalla_fase.c (con -DFASE1).
      Lo usan:    sistema/main.c (con -DFASE1 llama a fase1_ejecutar).

    HARDWARE QUE TOCA
      Ver pantalla_fase.c.

    VARIABLES: ninguna.
    → glosario: Fase, Director de fase, Motor de escenas
---------------------------------------------------------------------------------*/
#ifndef PANTALLA_FASE_H
#define PANTALLA_FASE_H

/* Carga la fase y se queda en su bucle para siempre (no vuelve). */
void fase1_ejecutar(void);

#endif /* PANTALLA_FASE_H */
