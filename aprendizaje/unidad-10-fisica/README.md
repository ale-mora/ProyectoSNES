# Unidad 10 — Física de plataformas: velocidad, aceleración, gravedad y salto

> **Lección válida, juego histórico.** Esta unidad enseña sobre el código de
> la Revisión 2 (plataformas, tag `plataformas-r2`); el juego actual es un
> beat 'em up ([`docs/diseno-beat-em-up.md`](../../docs/diseno-beat-em-up.md)).
> La física de plataformas, la rejilla de colisión y el tutorial se retiran
> del juego y quedan en ese tag. El ejercicio se hace en un clon fijado en él
> (receta en `ejercicio_10.md`); las cifras (`367 pruebas`, los fallos de
> `prueba_colision.c` y `prueba_recorrido.c`) son las de ese tag.
>
> **Qué se traslada al beat 'em up:** integrar (`posición += velocidad`), la
> aceleración y el frenado sirven para andar y correr por el carril (en x y en
> y), y la gravedad, el apogeo y el aterrizaje sirven para el salto en z, el
> ataque aéreo y los lanzamientos. Lo que no se traslada son los choques con
> paredes, techos y plataformas: el carril es un suelo plano limitado por
> columnas (spec §3).

**Objetivo:** que un personaje se mueva «como en un Mario»: arranca y frena
poco a poco, salta más alto cuanto más tiempo se mantiene B, cae cada vez más
deprisa hasta una velocidad máxima y aterriza exactamente en el suelo. Y saber
**afinar** esos números sin romper el nivel.

**Prerrequisitos:** 01 (punto fijo 12.4, `PF()`, `PF_ENTERO()`, por qué no
hay `/`), 02 (máquina de estados, `enum Estado`, `switch`), 05 (la `struct
Mando`: mantenidos frente a recién pulsados, el guion de mando) y, por debajo,
00, 03 y 04. **Práctica:** PC + sobre el juego real. **Tiempo orientativo:**
dos tardes.

Los choques contra paredes, techos y plataformas (`colision.c`, el retroceso
píxel a píxel, los dos pies) son de la unidad 20. Aquí se miran las cuentas
que los preceden: cuánto se mueve Tarou en cada frame.

## Conceptos

### Velocidad, aceleración e integrar

La **posición** es dónde está Tarou (`x_fp`, `y_fp`). La **velocidad** es
cuánto cambia la posición en un frame (`vx_fp`, `vy_fp`, en píxeles por frame,
→ glosario: Velocidad, aceleración y frenado). La **aceleración** es cuánto
cambia la velocidad en un frame. Todo en punto fijo 12.4 (unidad 01): 16 = 1 px.

**Integrar** es aplicar la velocidad a la posición una vez por frame:

```c
t->x_fp += t->vx_fp;     /* mover_x, src/juego/tarou.c */
t->y_fp += t->vy_fp;     /* mover_y */
```

Es la versión discreta de «la posición es la suma de las velocidades». Como
el juego va a 60 frames por segundo (unidad 04), `vx = 16` (1 px/frame) son
60 px por segundo. En la SNES la y crece **hacia abajo**: subir es `vy < 0`.

### Aceleración y frenado en el suelo

Si la velocidad saltara de 0 a máxima en un frame, el personaje se sentiría
de cartón. Tarou suma `ACEL_SUELO = 2` por frame hasta el máximo:

```
frame:   1  2  3  4  5  6  7  8  9 ...
vx_fp:   2  4  6  8 10 12 14 16 16 ...   (VEL_CAMINAR_MAX = 16: 8 frames hasta caminar)
x_fp:  +2 +4 +6 ... = tras 20 frames, 2+4+...+16 + 12·16 = 264 = 16,5 px
```

Esa cuenta (264) es literalmente la de `prueba_caminar` en
`tests/prueba_tarou.c`. El **frenado** (`frenar`) acerca `vx` a 0 de
`FRENADO_SUELO = 2` en 2 sin pasarse; y `acelerar` también **frena** cuando
`vx` supera el máximo nuevo (soltar Y corriendo: de 32 baja a 16 de 2 en 2, sin
cortar en seco). Las dos funciones tratan las dos direcciones con el signo.

### Gravedad, apogeo y velocidad terminal

La **gravedad** es una aceleración vertical constante: cada frame en el aire
`vy += GRAVEDAD` (→ glosario: Gravedad). El salto da una velocidad inicial
hacia arriba (`VEL_SALTO = -96`: 6 px/frame) y la gravedad (`4`: 0,25 px/frame
cada frame) se la va comiendo. El **apogeo** es el punto más alto: el frame en
que `vy` deja de ser negativa. Allí el juego cambia de estado:
`ESTADO_SALTANDO → ESTADO_CAYENDO`.

```
altura (px)
 69 ┤           ●●●●●●            apogeo: frames 23-24 (vy -4 → 0)
    │       ●●●       ●●●
 48 ┤.....●..............●......  el muro del tutorial
    │   ●                   ●
    │  ●                     ●
  0 ┼●─────────────────────────●  aterriza en el frame 47
     1    10    20    30    40  47   frame
```

La altura máxima se puede calcular sin simular: con velocidad inicial `v` y
gravedad `g` (en dieciseisavos), sube durante `n = v/g` frames, y como la
gravedad ya actúa en el frame del despegue, la subida total es la suma
`(v−g) + (v−2g) + ... + 0 = g·n(n−1)/2`. Con −96 y 4: `4·24·23/2 = 1104`,
**1104/16 = 69 px**. La celda empieza en y = 144 y su punto más alto es
144 − 69 = **75**: el rango 73..77 que acepta `prueba_apogeo_y_altura`.

La **velocidad terminal** es un tope para la caída (`VEL_TERMINAL = PF(6)` =
96): sin él, una caída larga acabaría moviendo a Tarou más de una celda de
colisión (16 px) por frame y la colisión podría «saltarse» un suelo.

### Salto de altura variable: el corte

En los Mario el salto es más alto cuanto más rato se mantiene el botón. La
manera más simple (→ glosario: Salto de altura variable): si el jugador
**suelta B mientras sube** y todavía sube más deprisa que `VEL_SALTO_CORTE`
(−32), la velocidad se recorta a −32. No se anula: el salto «se apaga» con
suavidad. El paso 2 de `tarou_actualizar`:

```c
if (t->estado == ESTADO_SALTANDO && !m->saltar && t->vy_fp < VEL_SALTO_CORTE)
{
    t->vy_fp = VEL_SALTO_CORTE;         /* Soltar B recorta la subida (altura variable).*/
}
t->vy_fp += GRAVEDAD;
```

Fíjate en `m->saltar` (B **mantenido**) frente a `m->saltar_pulsado` (B
**recién pulsado**, unidad 05): el despegue usa el pulsado (un salto por
pulsación); el corte, el mantenido. Soltando B en el frame 2 la altura máxima
baja de 69 a **12,75 px** y aterriza en el frame 19 (demo, parte 4).

### Control aéreo

En el aire, la cruceta acelera con `ACEL_AIRE = 1` (la mitad que en el suelo)
y **no hay frenado**: sin dirección, se conserva la velocidad del despegue.
Correr y saltar alarga el salto horizontalmente: 47 frames en el aire a 2
px/frame son 94 px; caminando, 47. Es lo que decide el foso (abajo).

### El orden de las operaciones dentro del frame

`tarou_actualizar` (`src/juego/tarou.c`) hace, siempre en este orden:

```
1. transiciones según el estado (manejar_pie / manejar_agachado / actualizar_aire / ...)
   y aceleración o frenado en x            → saltar() pone vy = VEL_SALTO aquí
2. en el aire: corte del salto → vy += GRAVEDAD → tope VEL_TERMINAL → ¿apogeo?
3. mover_x: x += vx, recorte a 0..1952, paredes (unidad 20)
4. mover_y: y += vy; ¿cruza un suelo? aterrizar; ¿techo?
5. avanzar_animacion (unidad 11)
```

El orden **cambia los números**. Como el paso 2 va después del 1, en el frame
del despegue ya actúa la gravedad: el primer `vy` no es −96 sino **−92**
(`prueba_despegue` lo comprueba: `VEL_SALTO + GRAVEDAD`). Y como x se resuelve
antes que y, una esquina nunca engancha a Tarou (unidad 20). Cambiar el orden
es cambiar el juego: todas las cuentas de las pruebas lo dan por fijo.

### Aterrizar

`mover_y`, bajando, mira si los pies (`y + ALTO_CELDA`, la última fila de la
celda) **cruzaron** el borde de arriba de una celda que sostiene; si es así,
coloca a Tarou exactamente encima (`y_fp = PF(fila − ALTO_CELDA)`) y llama a
`aterrizar`, que pone `vy = 0`, `en_suelo = 1` y elige estado con el mando:
Abajo → `AGACHADO`; con `vx ≠ 0` → `CORRIENDO` o `CAMINANDO`; si no,
`PARADO`. Colocar **exactamente** es importante: sin eso los pies acabarían
unos píxeles dentro del suelo, distintos en cada salto.

### Cajas: cuerpo y golpe

Una **caja** (`Caja`, en `tarou.h`) es un rectángulo en píxeles del mundo
(→ glosario: Caja de colisión / caja de golpe). Tarou tiene dos, **medidas
sobre el arte** de Pixellab (el perro ocupa unos 64×58 px dentro de la celda
de 96×64; `docs/diseno-demo-jugable.md` §5):

```
celda 96x64          cuerpo de pie: x 24..76, y 14..64 (52x50, sin la cola)
┌──────────────────┐  agachado:     y 28..64 (52x36)
│     ┌──────┐ ▒▒▒ │  golpe (solo ATACANDO): x 60..88, y 20..36 (28x16),
│     │cuerpo│ ▒▒▒ │  del morro a la punta de la espada; mirando a la
│     │      │     │  izquierda, x 8..36 (96 − 88 = 8)
└─────┴──────┴─────┘
```

`tarou_caja_cuerpo` y `tarou_caja_golpe` las calculan con constantes (`CUERPO_X`,
`GOLPE_X`...): ninguna cuenta por frame más allá de sumas. La caja del cuerpo
no es la celda: si fuera la celda, Tarou chocaría con el aire que rodea al
perro. Por eso se miden sobre el arte y se reajustan si el arte cambia.

### Afinar constantes «a afinar»

Las constantes de `tarou.h` dicen «a afinar en Mesen con el HUD». Afinar es
un bucle: cambiar un número, ver el efecto, comprobar que nada se rompe. Las
tres herramientas del proyecto:

1. **El PC** (`demo_10_salto.c`, las pruebas): la medida exacta.
2. **Las pruebas del nivel real**: `tests/prueba_colision.c` (cada obstáculo
   exige su movimiento) y `tests/prueba_recorrido.c` (el tutorial entero,
   jugado por el guion) dicen si un cambio hace imposible un obstáculo.
3. **Mesen con el HUD** (SELECT): la fila `Y n  VY v` enseña la posición y la
   velocidad. Cada fila del HUD se refresca cada 8 frames (y alguna espera si
   el frame va cargado: `HUD_LINEA_MAX`), así que una captura enseña una `Y`
   **cerca** del mínimo, no el mínimo exacto. La medida exacta es la del PC;
   Mesen corrobora que se siente bien.

## La demo: `demo_10_salto.c`

Simula el salto con **el mismo orden** que `tarou_actualizar` (gravedad antes
de integrar, también en el despegue) y lo comprueba contra las pruebas:

```
cd E:\ProyectoSNES\aprendizaje\unidad-10-fisica
gcc -std=c99 -Wall -Wextra -o demo_10.exe demo_10_salto.c
.\demo_10.exe
```

Salida (`capturas/salida_demo_10_salto.txt`, resumen):

```
VEL_SALTO  -96 GRAVEDAD 4: apogeo en el frame 23, altura  69.00 px (y_fp min 1200, y min  75 px), aterriza en el frame 47
    en el aire 47 frames: caminando 47 px, corriendo  94 px | muro de 48 px: SI | foso (68 px en el aire) corriendo: SI
VEL_SALTO  -80 GRAVEDAD 4: apogeo en el frame 19, altura  47.50 px (y_fp min 1544, y min  96 px), aterriza en el frame 39
    en el aire 39 frames: caminando 39 px, corriendo  78 px | muro de 48 px: NO | foso (68 px en el aire) corriendo: SI
VEL_SALTO -112 GRAVEDAD 4: apogeo en el frame 27, altura  94.50 px (y_fp min  792, y min  49 px), aterriza en el frame 56
VEL_SALTO -120 GRAVEDAD 6: apogeo en el frame 19, altura  71.25 px (y_fp min 1164, y min  72 px), aterriza en el frame 40
```

Las cifras del juego coinciden con las pruebas: `y` mínima 75 (rango 73..77
de `prueba_apogeo_y_altura`), aterrizaje en el frame 47 (`prueba_aterrizaje`),
salto corto que aterriza antes del frame 20 (`prueba_salto_corto`). Si una
línea de la demo no coincidiera con una prueba, lo que está mal es la demo.

Las columnas «muro» y «foso» son la versión de bolsillo de las pruebas del
nivel: el muro mide 48 px y el foso exige recorrer 68 px en el aire (los dos
pies están a 28 px y el foso mide 96: `docs/diseno-tutorial-movimiento.md`
§2). Son aproximadas a propósito (no simulan las paredes); las pruebas reales
son las de `prueba_colision.c`.

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| Las constantes (`VEL_CAMINAR_MAX`, `ACEL_SUELO`, `GRAVEDAD`, `VEL_SALTO`, `VEL_SALTO_CORTE`, `VEL_TERMINAL`) | `src/juego/tarou.h` |
| La `struct Tarou` (`x_fp`, `vy_fp`, `en_suelo`...) y la `Caja` | `src/juego/tarou.h` |
| `acelerar`, `frenar` | `src/juego/tarou.c` |
| `saltar` (vy = VEL_SALTO, conserva vx) | `src/juego/tarou.c` |
| `actualizar_aire` (control aéreo sin frenado) | `src/juego/tarou.c` |
| Paso 2 de `tarou_actualizar`: corte, gravedad, terminal, apogeo | `src/juego/tarou.c`, `tarou_actualizar` |
| Integrar: la primera línea de `mover_x` y de `mover_y` (lo demás, unidad 20) | `src/juego/tarou.c` |
| `aterrizar` (estado según Abajo y vx) | `src/juego/tarou.c` |
| `tarou_caja_cuerpo`, `tarou_caja_golpe` y las constantes `CUERPO_*`, `GOLPE_*` | `src/juego/tarou.c` |
| Las cuentas, en pruebas sobre un escenario plano (la fila 13 de una rejilla hecha en la prueba, `colision_usar(plano)`) | `tests/prueba_tarou.c`: `prueba_caminar`, `prueba_despegue`, `prueba_apogeo_y_altura`, `prueba_salto_corto`, `prueba_velocidad_terminal`, `prueba_aterrizaje*`, `prueba_control_aereo`, `prueba_salto_corriendo`, `prueba_agacharse` (cajas) |
| Cada obstáculo exige su movimiento (muro con salto corto y largo, foso caminando y corriendo) | `tests/prueba_colision.c`: `prueba_muro`, `prueba_foso` |
| El tutorial entero con el guion | `tests/prueba_recorrido.c`, `src/juego/guion_tutorial.c` |
| La tabla de constantes y el orden del frame | `docs/diseno-demo-jugable.md` §5 |
| Las filas `Y` / `VY` del HUD | `src/juego/hud_debug.c` |

## Trampas

- **Las pruebas de recorrido usan la rejilla REAL del nivel**
  (`res/nivel1/nivel1_datos.h`). Un cambio de física que parece inocente
  (un salto 1,5 px más bajo) rompe `prueba_muro` y `prueba_recorrido`. No es
  un fallo de las pruebas: es el nivel diciendo que el muro ya no se puede
  saltar. Se arregla la física o el nivel, no la prueba.
- **`tests/tarou_anim.h` es una copia fija** (unidad 02). Las cuentas de
  frames de las animaciones en las pruebas (desenvainar 4 × 5...) son las de
  esa copia, no las del arte real. Afecta a esta unidad si una prueba nueva
  espera a que termine una animación; se ve a fondo en la 11.
- **`PF_ENTERO` solo para valores ≥ 0.** Un salto muy alto desde una
  plataforma puede llevar la celda por encima de la pantalla (y < 0) y
  `PF_ENTERO` da un número enorme: Tarou «moría» en lo alto del salto. Por eso
  `partida.c` compara en punto fijo (`t->y_fp >= PF(Y_CAIDA)`). Si subes mucho
  `VEL_SALTO`, vigílalo.
- **El orden del frame.** La gravedad actúa en el frame del despegue. Una
  simulación que integre antes de sumar la gravedad (`y += vy; vy += g`) da
  75 px de altura y aterriza en el frame 49, no 69 y 47: no coincidiría con
  ninguna prueba.
- **Cambiar un `.h` y no hacer `make clean`.** `snes_rules` no sigue las
  cabeceras: tras tocar `tarou.h`, la ROM conserva los valores viejos si no se
  limpia (CLAUDE.md). Las pruebas de PC sí se recompilan (`tests/Makefile`
  depende de las cabeceras).
- **Velocidad terminal mayor de 16 px/frame** (256 en 12.4): la colisión por
  celdas de 16 px podría saltarse un suelo. Hoy es 6 px/frame.

## Ejercicio y criterio de aprobación

Enunciado completo en `ejercicio_10.md`; solución en `solucion/`.

- **Parte A — romper el muro:** `VEL_SALTO = -80` y explicar los 6 fallos de
  las pruebas (salida real en `capturas/pruebas_parteA_salto_flojo.txt`).
- **Parte B — un salto más rápido a la misma altura:** `GRAVEDAD 6` y
  `VEL_SALTO -120`; actualizar las cuentas de `tests/prueba_tarou.c`, ver las
  367 pruebas en verde (`capturas/pruebas_solucion.txt`), compilar la ROM que
  se juega sola y medir la altura en Mesen con el HUD.

**Aprobación:** `tests/pruebas.exe` en verde con las constantes nuevas
(`367 pruebas, 0 fallos`), la explicación escrita de la parte A y una captura
del HUD de la ROM de prueba con la `Y` cerca del nuevo mínimo (72) en el salto
del muro. Al terminar, deshacer los cambios en tu copia (`git restore`) o
dejarlos en una rama propia.

## Lo que viene

- **Unidad 11:** qué cuadro de animación se ve en cada estado y a qué ritmo.
- **Unidad 20:** `mover_x`/`mover_y` enteros: paredes, plataformas, techos,
  los dos pies y el túnel.
