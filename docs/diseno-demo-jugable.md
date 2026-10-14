# Diseño — Demo jugable: Tarouh cachorro camina por una pequeña etapa

**Proyecto:** _Tarouh: A Dog's Journey_ · **Etapa del plan:** 2 (Prototipo),
primer paso: caminar + cámara · **Fecha de diseño:** 2026-10-14 · **Estado:**
diseño aprobado en conversación; pendiente de revisión de este documento y del
plan de implementación.

Este documento especifica una demo mínima y jugable: al pulsar Start en el
título, el jugador controla a Tarouh cachorro, que camina a izquierda y derecha
por una etapa de 512 px (dos pantallas) con scroll. Sigue el mismo formato que
`docs/diseno-secuencia-arranque.md` y, como aquel, es un **documento vivo**: los
valores marcados «a afinar» se ajustan tras verlos en Mesen.

---

## 1. Propósito y alcance

- **Qué es:** el primer contacto con gameplay. Entrada del mando, movimiento con
  punto fijo, animación por estados y cámara con scroll. Es la base sobre la que
  la Etapa 2 añadirá salto y colisiones.
- **Qué no es:** no hay salto, colisión con el escenario, enemigos, Bárbara,
  audio nuevo ni etapas de vida (adulto/viejo). Ver [§10](#10-fuera-de-alcance).
- **Criterio de éxito:** una ROM que, desde el título, entra al nivel con Start;
  el perro camina hacia ambos lados con animación y espejo correctos; la cámara
  lo sigue y se detiene en los extremos del escenario; Start vuelve al título.
  Sin glitches gráficos, a 60 FPS.
- **Restricción de estilo del proyecto:** cada línea de código lleva su
  comentario en español (el autor aprende C y SNES con este código).

## 2. Decisiones tomadas

| # | Decisión | Elegido | Motivo |
|---|---|---|---|
| 1 | Tamaño del sprite | **32×32** (par de tamaños `OBJ_SIZE32_L64`, se usa el pequeño) | Los 12 cuadros caben en VRAM con carga estática (192 de 512 tiles). Más nivel visible. Deja abierto un adulto en 64×64 |
| 2 | Tipo de escenario | **Suelo plano con scroll**, 512 px | Reutiliza la técnica de mapa 64×32 + scroll del título. Sin salto ni colisión todavía |
| 3 | Construcción del escenario | **Fondo de imagen + `suelo_en(x)`** | Pocas piezas nuevas. El suelo es una función, no parte de la imagen: añadir desniveles solo cambia su interior |
| 4 | Dónde se enchufa | `PANTALLA_NIVEL1` **sustituye** a `PANTALLA_PROXIMAMENTE` | Esa pantalla ya se describe como «el enchufe del gameplay» |
| 5 | Salida del nivel | **Start vuelve al título**, con fundido | Es lo que hacía «coming soon» |
| 6 | Movimiento | Punto fijo **12.4** con aceleración y frenado suaves | Ver [§5](#5-movimiento-y-cámara) |
| 7 | Arte del perro | Sprites de **Pixellab**; mientras tanto, **marcador numerado** | Permite construir y probar el código sin esperar al arte |

Alternativa descartada: tileset + mapa de tiles en Tiled (estilo LikeMario).
Es la base real de colisiones, pero exige un tileset nuevo y un conversor TMX, y
retrasaría la demo. Se retomará cuando haya desniveles (§10).

## 3. Especificación de los sprites (para Pixellab)

**Obligatorias por el hardware**

| Aspecto | Especificación |
|---|---|
| Tamaño de cuadro | **32×32 px**, igual en todas las animaciones |
| Colores | **Máximo 15 opacos + transparente**, **una sola paleta** para todo el perro |
| Diferencia entre colores | Bien distintos: la SNES usa 5 bits por canal y los tonos casi iguales se funden |
| Transparencia | PNG RGBA con alfa **0 o 255**, sin semitransparencias |
| Contenido | Solo el perro: sin sombra proyectada, sin suelo, sin fondo |
| Dirección | Una sola, **mirando a la derecha**, vista lateral. La izquierda se obtiene con `hflip`. Sin luz fuerte desde un lado |

**Obligatorias para animar bien**

- Las patas tocan **la misma fila** en todos los cuadros (ideal: la 31, la última).
- **Caminar en el sitio:** el cuerpo no avanza dentro de la celda; el desplazamiento lo pone el código.
- **Bucle limpio:** número par de cuadros y el último no repite el primero.
- Cachorro de unos 24–28 px de largo y 16–20 px de alto, con 2–3 px de margen.
- Contorno oscuro de 1 px, sombreado de 2–3 tonos por color, sin dithering ni antialiasing.

**Animaciones:** reposo de 4 cuadros y caminar de 8 (valen 6). **Entrega:** una
tira horizontal por animación, o cuadros sueltos numerados en orden.

**Flujo en Pixellab:** _Create character_ con cámara _Side_ y lienzo 32×32 →
reducir a ≤ 15 colores **antes** de animar → _Animate with text (new)_ con el
personaje como referencia (4–16 cuadros pares), limitando los colores a la
paleta de esa imagen → retocar la fila de las patas a mano.

**No verificado en su documentación** (comprobar en la interfaz): si hay
plantilla de cuadrúpedo (perro) y si la animación sale «en el sitio». La
animación por esqueleto probablemente va mejor con humanoides; para un perro,
usar texto.

## 4. Arquitectura

```
main.c → secuencia.c ── PANTALLA_TITULO ──Start──▶ PANTALLA_NIVEL1
                                                     │ pantalla_nivel1.c  (orquesta)
                                                     ├─ perro.c   (estado, animación, OAM)
                                                     └─ nivel.c   (suelo_en(x), cámara)
```

**Piezas nuevas**

- `src/pantalla_nivel1.c` — `pantalla_nivel1_iniciar()` y `_actualizar()`, con el patrón de todas las pantallas.
  - Al iniciar: modo de vídeo, fondos, sprite, posición inicial, cámara en 0, cielo HDMA, fundido de entrada.
  - Por frame: mando → `perro_actualizar` → cámara → scroll de las dos capas → `perro_dibujar`. Start: fundido de salida, corta el HDMA, oculta el sprite y vuelve al título.
- `src/perro.c/.h` — el jugador. Interfaz prevista:
  - `perro_cargar()` copia tiles y paleta a VRAM/CGRAM (una vez al entrar).
  - `perro_iniciar(x)` fija la posición inicial y el estado.
  - `perro_actualizar(pad)` lee el mando y avanza física y animación.
  - `perro_dibujar(camara_x)` escribe la entrada de la OAM.
  - `perro_x()` devuelve la posición en píxeles del mundo.
- `src/nivel.c/.h` — constantes del escenario, `nivel_suelo_en(x)` y `nivel_camara_para(x)`.

**Cambios en lo existente:** `pantallas.h` (enum y prototipos), `secuencia.c`
(los dos `switch`), `pantalla_titulo.c` (Start lleva a `PANTALLA_NIVEL1`),
`Makefile` (reglas y `ASSETS`), `data.asm` (sección del perro y del fondo
lejano). `pantalla_proximamente.c` se elimina.

**Volver al título es seguro:** `pantalla_titulo_iniciar` ya llama a
`personajes_cargar()`, así que los tiles del perro que pisen la VRAM de sprites
no estorban.

## 5. Movimiento y cámara

- **Coordenadas:** el perro vive en coordenadas del mundo, 0 a 480 px (el
  escenario mide 512 y la celda 32). Los píxeles que usa la OAM y el scroll son
  siempre la **parte entera** (`x_fp >> 4`): sprite y fondo comparten el mismo
  entero y nunca se desfasan un píxel.
- **Punto fijo 12.4 en `s16`:** 12 bits de entero y 4 de fracción (1/16 px).
  - _Corrección respecto a la conversación:_ se habló de 8.8, pero 8 bits de
    entero solo llegan a 255 px y el escenario mide 512. Con 12.4 caben hasta
    2047 px y todo sigue siendo aritmética de 16 bits, nativa del 65816.
- **Constantes iniciales (a afinar en Mesen):**
  - `VEL_MAX` = 16 (1,0 px/frame, 60 px/s). Cruzar el nivel entero tarda ~8,5 s.
  - `ACELERACION` = 2 (~8 frames hasta la velocidad máxima).
  - `FRENADO` = 2 (~8 frames hasta parar).
  - 5 frames por cuadro al caminar (ciclo de 40 frames ≈ una zancada de 40 px, para que las patas no patinen).
  - Reposo: ~15 frames por cuadro.
- **Estados:** `PARADO` y `CAMINANDO`. La dirección (`derecha`/`izquierda`) decide el `hflip`. Se pasa a `CAMINANDO` al pulsar una dirección y a `PARADO` cuando la velocidad llega a 0.
- **Límites:** `x` se recorta a 0..480.
- **Cámara:** `camara = x_pix + 16 - 128`, recortada a 0..256 (`ANCHO_NIVEL` − `ANCHO_PANTALLA`). El perro se mueve hasta el centro, la cámara lo sigue y, al final, el perro llega al borde derecho. Sin suavizado (YAGNI).
- **Posición en pantalla:** `x_pantalla = x_pix − camara`.
- **Altura:** `y_celda = nivel_suelo_en(x_pix + 16) − 32` (la última fila del cuadro es la planta de las patas). `nivel_suelo_en` devuelve hoy una **constante** (≈ 208–216, a medir sobre el fondo).

## 6. Sprite y animación

- **VRAM:** los 12 cuadros (192 tiles) se cargan una vez en `VRAM_SPRITES` (0x0000) con `oamInitGfxSet(..., OBJ_SIZE32_L64)`. La paleta va a la de sprites 0 (CGRAM 128).
- **OAM:** una sola entrada (id 0), tamaño `OBJ_SMALL` (32×32), prioridad 3 (delante de los fondos), `hflip` según la dirección.
- **Numeración de tiles (a verificar al convertir):** `gfx4snes -s 32` empaqueta los bloques en filas de 128 px, cuatro por fila. El cuadro `n` empieza en el tile `(n/4)*64 + (n%4)*4`. Es la misma cuenta que ya se verificó para los bloques de 64 en `personajes.c`.
- **Cuadros:** reposo = 0–3, caminar = 4–11. Constantes de inicio, cantidad y duración por animación.

## 7. Fondo

- **Modo:** Mode 1, solo BG0 y BG1 (sin capa de texto). Se replica el patrón de `pantalla_titulo_iniciar` para activar las capas.
- **Capa cercana (BG0):** reutiliza los arbustos del título, `cerca_*`, de 512 px en formato `SC_64x32`, paleta 1. Scroll 1:1 con la cámara.
- **Capa lejana (BG1):** montañas y pinos, paleta 2, mapa `SC_64x32` en `VRAM_MAPA_B` (0x7400–0x7BFF). Scroll a la mitad de la cámara (`camara >> 1`, rango 0–128).
  - `fondo_lejos.png` del título **lleva el logo horneado** y mide 256 px, así que no sirve tal cual.
  - `scripts/preparar_fondo_nivel.py` deriva `res/nivel1/fondo_lejos.png`: borra el logo (los píxeles pasan al índice 0, transparente) y lo duplica en espejo hasta 512 px, el mismo truco que ya usa la capa cercana. Se convierte con `-y`.
- **Cielo:** el degradado HDMA del título (`hdmaGradcieloList`, canal 6), que se corta al salir con `setModeHdmaReset(0)`.
- **Presupuesto de tiles:** la cercana usa 294 y la lejana ~262 (a confirmar con el espejo), de 512 disponibles por capa.

## 8. Pipeline de assets

- `scripts/generar_placeholder_perro.py`: 12 cuadros de 32×32 con el número de cuadro, como tira `res/personajes/tarouh_cachorro.png`. Se sustituye por el arte de Pixellab sin tocar código.
- `scripts/preparar_sprites.py`: hoy cuantiza y recentra cada tira por separado, lo que daría paletas distintas y cuadros desalineados entre reposo y caminar. Se le añade una **paleta compartida entre tiras** y un modo que **no recentra** (para arte ya alineado).
- `Makefile`: regla `gfx4snes -s 32 -o 16 -u 16 -p` para el perro, regla `-y` para el fondo lejano, y ambos en `ASSETS`.
- `data.asm`: sección `.rodata_perro` (`perro_til`, `perro_til_end`, `perro_pal`) y sección del fondo lejano del nivel. Cada una cabe holgada en un banco de 32 KB.
- Tras tocar `pantallas.h` hay que hacer `make clean` antes de `make` (el SDK no rastrea cabeceras).

## 9. Verificación

No hay suite de tests. Se compila con el comando de PowerShell del `CLAUDE.md` y se prueba en Mesen:

1. Compila sin errores ni avisos nuevos.
2. Desde el título, Start entra al nivel con fundido.
3. El perro anda a derecha e izquierda; el espejo es correcto; los 12 cuadros del marcador salen en orden.
4. La cámara lo sigue, se detiene en 0 y en 256, y el perro llega a los bordes.
5. Las dos capas se desplazan a distinta velocidad sin costuras visibles.
6. Los pies quedan sobre el suelo en todos los cuadros.
7. Start vuelve al título y este se ve bien (sprites del lobo, HDMA, parallax).
8. Capturas con `scripts/capturas_mesen.lua`.

Documentación a actualizar: `docs/diseno-secuencia-arranque.md` (la pantalla 7),
`docs/glosario-snes.md` (punto fijo, cámara), `docs/plan-desarrollo.md` (avance
de la Etapa 2) y `CLAUDE.md`.

## 10. Fuera de alcance

Salto, caída y gravedad · colisiones con el escenario · desniveles, tileset y
Tiled · enemigos e ítems · Bárbara y el cambio de personaje · etapas de vida
adulta y vieja · audio nuevo · cámara suavizada · HUD.

## 11. Riesgos y puntos a verificar

- **Numeración de tiles de `-s 32`:** se comprueba convirtiendo el marcador numerado antes de fiarse de la fórmula.
- **Altura del suelo:** la constante se mide sobre el fondo en Mesen; `nivel_suelo_en` aísla el ajuste.
- **Tiles de la capa lejana:** el espejo podría superar lo previsto; el límite son 512.
- **Arte de Pixellab:** que el perro salga bien con 15 colores y «en el sitio» no está garantizado; puede requerir varios intentos y retoques.
- **Cambios de `pantallas.h`:** olvidar `make clean` deja objetos antiguos con el enum viejo.
