/*---------------------------------------------------------------------------------
    enemigos.h — Enemigos del nivel (Etapa 3).

    PROPÓSITO
      PENDIENTE: Etapa 3 — cargar, mover, dibujar y golpear a los enemigos; el primero será el lobo samurái (res/personajes/lobo_samurai.png), con el mismo motor de sprites dinámicos que Tarou.
      Este archivo es un STUB: compila, reserva el sitio del subsistema y documenta
      el contrato previsto. No contiene lógica; la única función real,
      enemigos_pendiente(), existe para que el enlazador vea el objeto.

    SE CONECTA CON
      Incluirá: sistema/tipos.h, juego/tarou.h (Caja), juego/animaciones.h.
      Lo usarán: pantallas/pantalla_nivel1.c (actualizar y dibujar por frame; golpe de la espada).

    HARDWARE QUE TOCA
      OAM y VRAM por el motor de sprites dinámicos (como tarou_sprite.c).

    VARIABLES (globales y estáticas de este archivo)
      ninguna todavía.

    → glosario: Stub, Enemigo, Metasprite, Caja de colisión / caja de golpe
---------------------------------------------------------------------------------*/
#ifndef ENEMIGOS_H
#define ENEMIGOS_H

#include "sistema/tipos.h"

/* Contrato previsto (Etapa 3):
void enemigos_cargar(u8 nivel);              -- posiciones iniciales del nivel
void enemigos_actualizar(void);              -- un frame de cada enemigo vivo
void enemigos_dibujar(u16 camara);           -- metasprites en pantalla
u8   enemigos_golpe(const Caja *golpe);      -- 1 si la espada alcanzó a alguno */

/* Única función real del stub: no hace nada. Se quitará al implementar el módulo. */
void enemigos_pendiente(void);

#endif /* ENEMIGOS_H */
