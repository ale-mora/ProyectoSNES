/*---------------------------------------------------------------------------------
    secuencia.h — Interfaz de la máquina de estados que encadena las pantallas.

    Quien incluye este header puede: arrancar la secuencia, avanzar un frame,
    cambiar de pantalla, y consultar cuántos frames lleva la pantalla actual.
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

#endif /* SECUENCIA_H */
