/*---------------------------------------------------------------------------------
    objetos.h — Objetos recogibles del nivel (Etapa 3).

    PROPÓSITO
      PENDIENTE: Etapa 3 — ítems que el protagonista recoge al tocarlos (comida, llaves...), con su sprite y su efecto.
      Este archivo es un STUB: compila, reserva el sitio del subsistema y documenta
      el contrato previsto. No contiene lógica; la única función real,
      objetos_pendiente(), existe para que el enlazador vea el objeto.

    SE CONECTA CON
      Incluirá: sistema/tipos.h, juego/tarou.h (Caja).
      Lo usarán: pantallas/pantalla_nivel1.c, hud/hud.c (contador de objetos).

    HARDWARE QUE TOCA
      OAM y VRAM por el motor de sprites dinámicos.

    VARIABLES (globales y estáticas de este archivo)
      ninguna todavía.

    → glosario: Stub, Caja de colisión / caja de golpe
---------------------------------------------------------------------------------*/
#ifndef OBJETOS_H
#define OBJETOS_H

#include "sistema/tipos.h"

/* Contrato previsto (Etapa 3):
void objetos_cargar(u8 nivel);
void objetos_actualizar(void);
u8   objetos_recoger(const Caja *cuerpo);    -- 1 si el cuerpo del protagonista tocó alguno */

/* Única función real del stub: no hace nada. Se quitará al implementar el módulo. */
void objetos_pendiente(void);

#endif /* OBJETOS_H */
