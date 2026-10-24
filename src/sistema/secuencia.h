/*---------------------------------------------------------------------------------
    secuencia.h — Interfaz de la máquina de estados que encadena las pantallas.

    PROPÓSITO
      Quien incluye este header puede arrancar la secuencia, avanzar un frame,
      ejecutar la parte posterior al VBlank, cambiar de pantalla y consultar
      cuántos frames lleva la pantalla actual.

    SE CONECTA CON
      Incluye:  sistema/pantallas.h (el tipo Pantalla).
      Lo usan:  sistema/main.c, sistema/secuencia.c y todas las pantallas.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES (globales y estáticas de este archivo)
      frames_en_pantalla  u16  0..65535 frames  declarada extern: VIVE en secuencia.c

    → glosario: Máquina de estados, extern
---------------------------------------------------------------------------------*/
#ifndef SECUENCIA_H
#define SECUENCIA_H

#include "sistema/pantallas.h"   /* Necesitamos el tipo Pantalla (el enum) definido allí. */

/* Cuántos frames lleva activa la pantalla actual. Se pone en 0 en cada cambio
   de pantalla y sube 1 por frame. La variable VIVE en secuencia.c; "extern"
   avisa a los demás archivos de que existe y pueden leerla. → glosario: extern */
extern u16 frames_en_pantalla;

/* Arranca la secuencia en la pantalla indicada (se llama una vez desde main). */
void secuencia_iniciar(Pantalla primera);

/* Ejecuta un frame de la pantalla actual (se llama una vez por vuelta del bucle). */
void secuencia_actualizar(void);

/* Abandona la pantalla actual y entra en `nueva` (llama a su función _iniciar). */
void secuencia_cambiar(Pantalla nueva);

/* Trabajo de la pantalla actual que debe hacerse JUSTO después del VBlank (por
   ejemplo, copiar a la VRAM los tiles del sprite dinámico del nivel). main.c
   la llama una vez por vuelta, inmediatamente después de WaitForVBlank(). Las
   pantallas que no lo necesitan no definen _vblank y aquí no se hace nada. */
void secuencia_vblank(void);

#endif /* SECUENCIA_H */
