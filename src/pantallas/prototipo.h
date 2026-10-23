/*---------------------------------------------------------------------------------
    prototipo.h — El prototipo de combate: Tarou y un ninja en un carril (solo con
    -DPROTOTIPO).

    PROPÓSITO
      Es la escena de la Etapa 2 (entregable 3, docs/diseno-beat-em-up.md): un
      protagonista y un ninja de prueba, con sus golpes resueltos por la lógica
      pura (luchador, ninja, combate). Sustituye al nivel 1 en la ROM de
      prototipo. Se queda en su bucle para siempre (no vuelve).

    SE CONECTA CON
      Implementa: pantallas/prototipo.c (con -DPROTOTIPO).
      Lo usan:    sistema/main.c (con -DPROTOTIPO llama a prototipo_ejecutar).

    HARDWARE QUE TOCA
      Ver prototipo.c.

    VARIABLES: ninguna.
    → glosario: Prototipo de combate, Carril
---------------------------------------------------------------------------------*/
#ifndef PROTOTIPO_H
#define PROTOTIPO_H

/* Carga la escena y se queda en su bucle para siempre (no vuelve). */
void prototipo_ejecutar(void);

#endif /* PROTOTIPO_H */
