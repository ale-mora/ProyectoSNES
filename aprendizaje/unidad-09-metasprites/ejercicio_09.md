# Ejercicio 09 — Una animación más, y medir las piezas

Cuatro partes. La A y la B son sobre la mini-ROM (`rom/`), la C sobre el juego
real sin cambiar nada y la D, opcional, en un clon del proyecto. La solución de
la A está en `solucion/rom/` (míralo **después**).

## Parte A — Tarou duerme con Abajo (mini-ROM)

`rom/res/tarou_dormido.png` ya está copiado (es la tira del juego: 9 cuadros
de 96×64, 864×64 px). Añádela como tercera animación.

- **TODO 1 (`Makefile`).** Añade `res/tarou_dormido.pic` a la lista de
  `bitmaps:`. La regla patrón `res/%.pic: res/%.png` ya sabe convertirla.
- **TODO 2 (`Makefile`).** Añádela también a `data.obj:` (si no, al cambiar el
  PNG `data.asm` no se reensambla y la ROM sigue con los tiles viejos).
- **TODO 3 (`data.asm`).** Una sección nueva `.rodata_tarou_dormido`
  `superfree` con la etiqueta `tarou_dormido_til` y su `.incbin`. Una sección
  por tira: 9 cuadros = 27 KB, cabe en un banco de 32 KB; dos tiras juntas no.
- **TODO 4 (`src/main.c`).** `#include "tarou_dormido_meta.inc"` y el
  `extern char tarou_dormido_til;`.
- **TODO 5 (`src/main.c`).** `ANIM_DORMIDO 2`, `DORMIDO_CUADROS 9`,
  `DORMIDO_FRAMES 20` (el ritmo de `src/juego/animaciones.c`) y su rama en el
  paso 5: avanzar el cuadro, elegir `tarou_dormido_metasprites[cuadro]` y
  `tarou_dormido_til`.
- **TODO 6 (`src/main.c`).** En el paso 4, `KEY_DOWN` pone `ANIM_DORMIDO`
  (sin moverse), con `cuadro = 0` y `frames = 0` al entrar. Añade la línea
  `DOWN: SLEEP` a las instrucciones (texto en pantalla en inglés).
- **TODO 7 (`src/main.c`).** Que la línea de estado diga también cuántas
  piezas tiene el cuadro: una función `piezas(tabla)` que cuente hasta
  `dx == FIN_TABLA` (como `metasprite_piezas`), y el texto
  `ANIM SLEEP FRAME n PIECES p`. Sin `?:` con cadenas: un `if`/`else` por
  animación. Cada `u8` que pases a un `%d` va con `(u16)`: tcc816 empuja un
  `u8` como un solo byte y `%d` lee dos (README, «La primera versión de esta
  mini-ROM salía rota»). Y no llames a `texto` en el frame en que cambia el
  cuadro: la mini-ROM ya la retrasa un frame con `texto_pendiente`.

Compila con la orden del README (debe decir `No warnings or errors found.`).
Antes de abrir Mesen, **predice**: mira `rom/res/tarou_dormido_meta.inc` (lo
genera `make`) y apunta qué cuadros tienen menos de 6 `METASPR_ITEM`.

**Lo que debe verse:** con Abajo, Tarou tumbado respirando, entero y sin
restos del Tarou de pie; en los cuadros 0, 1 y 8, `PIECES 4`; en los demás,
`PIECES 6`; `VBLANK LINES` por debajo de 37. Si mirabas a la izquierda antes de
pulsar Abajo, duerme mirando a la izquierda (el espejo vale para cualquier
tabla).

## Parte B — Medir en el Sprite Viewer (mini-ROM)

Abre la ROM en Mesen y `Debug → Sprite Viewer` (unidad 16 lo explica a fondo).

1. Con Tarou en reposo, las entradas 0..5 son de 32×32 (*Large*). Anota sus
   x: deben ser `x`, `x+32`, `x+64` (dos veces, con y e y+32).
2. Camina a la izquierda: las mismas 6 entradas con el volteo horizontal
   marcado y las x intercambiadas (la pieza del bloque de la izquierda está
   ahora a la derecha).
3. Con Abajo, espera al cuadro 0 de dormido: solo 4 entradas visibles; las
   otras dos, escondidas por `oamInitDynamicSpriteEndFrame`.

## Parte C — El motor propio, en el juego real (sin modificar)

Con la ROM normal del juego (`ProyectoSNES.sfc`, la que deja `run.bat`) en el
nivel 1:

1. Sprite Viewer: las entradas 0..5 son Tarou. Camina: su número de tile
   alterna entre 0, 4, 8, 64, 68, 72 y 128, 132, 136, 192, 196, 200 **cada vez
   que cambia el cuadro**. Es el doble búfer: el cuadro nuevo se escribió en
   la otra celda.
2. Pulsa SELECT (tecla E) para el HUD. La fila `V a b`: anota `b` en varios
   frames. Debe quedar por debajo de 37 (hoy, 7 en frames normales y 17..20
   cuando sube media celda).
3. Responde por escrito: ¿por qué el tile de una pieza no depende de su
   `dtile`? (Pista: `tarou_sprite_dibujar`, paso 2, y adónde copia
   `metasprite_copias` cada bloque.)

## Parte D (opcional) — La trampa del `tcc__mul`, medida

En un clon propio (`git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\mio`),
nunca en el repositorio:

1. Quita el campo `u16 libre;` de `CopiaBloque` (`src/juego/metasprite.h`) y
   la línea `salida[copias].libre = 0;` de `metasprite.c`.
2. Genera el ensamblador con la receta de PLAN §6.4:
   `make src/juego/metasprite.asm` (para `tarou_sprite.asm` hacen falta antes
   los `.pic` de Tarou: `make res/personajes/tarou/tarou_reposo.pic` y las
   demás, o un `make` completo). **Mueve el `.asm` fuera de `src/` en
   seguida**: si se queda, `make` lo ensamblaría junto al `.c`.
3. Cuenta `tcc__mul` en cada uno (`Select-String -Pattern tcc__mul`).
   Después vuelve a poner `libre` y repite.

**Esperado** (`capturas/tcc_mul_copiabloque.txt`): con 8 bytes, 0 y 0; con 6,
3 en `metasprite.asm` y 5 en `tarou_sprite.asm` (cada `copias[copiadas]` de
`apuntar_copias` carga `#6` y llama a la rutina; con 8 bytes son tres `asl`).

## Aprobación

- La solución de la parte A compila sin avisos y en Mesen se ve a Tarou en las
  dos direcciones y dormido con `PIECES 4` (capturas).
- Las x de las 6 piezas (parte B) y la alternancia de tiles (parte C)
  anotadas, con el valor de `V` leído.
- (Opcional) los cuatro recuentos de la parte D.
