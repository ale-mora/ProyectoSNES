# Diseño — Demo jugable, Revisión 1: Tarou adulto en el nivel 1

**Proyecto:** _Tarou: A Dog's Journey_ · **Etapa del plan:** 2 (Prototipo) ·
**Revisión:** 1 · **Fecha de esta versión:** 2026-10-16 · **Estado:** diseño
acordado en sesión de preguntas (33 decisiones); el plan de implementación
está en `docs/plan-implementacion-revision-1.md`.

Este documento especifica el producto mínimo viable (MVP) jugable: desde el
título, Start entra al nivel 1, donde el jugador controla a **Tarou adulto**,
que camina, corre, salta, se agacha, desenvaina y usa la espada, y se duerme si
nadie toca el mando. Es un **documento vivo**: los valores marcados «a afinar»
se ajustan tras verlos en Mesen con el HUD de depuración.

> **Vocabulario.** Este documento usa los términos fijados en
> `docs/glosario-snes.md` → «Vocabulario del proyecto»: Etapa, Revisión,
> Nivel, Escenario, Pantalla, Protagonista, Jugador, Edad, Estado, Espada,
> Dirección, Stub. Si aquí aparece otra palabra para el mismo concepto, es un
> error.

## 0. Historial

| Versión | Fecha | Qué decía |
|---|---|---|
| v1 | 2026-10-14 | Tarou **cachorro** de 32×32 con carga estática de 12 cuadros; solo caminar y cámara. Reemplazada por completo. Sus especificaciones de Pixellab para el cachorro quedan en el historial de git (`3b30b64`). |
| v2 | 2026-10-16 | Tarou **adulto** de 64×64 como metasprite dinámico; ocho mecánicas; HUD de depuración; reorganización de `src/`; renombrado Tarouh → Tarou; ROM de 512 KB. |

---

## 1. Propósito y alcance

- **Qué es:** el primer contacto con gameplay real. Entrada del mando, física
  con punto fijo, máquina de estados del protagonista con trece estados,
  animación por estados con sprites dinámicos, cámara con scroll y parallax.
  Es la base sobre la que la Etapa 2 añadirá colisión con el escenario y la
  Etapa 3 enemigos.
- **Qué no es:** no hay colisión con tiles ni desniveles, ni enemigos, ni
  daño, ni Bárbara jugable, ni audio nuevo, ni HUD del juego (vida, ítems).
  Ver [§14](#14-fuera-de-alcance).
- **Criterio de éxito:** una ROM que, desde el título, entra al nivel con
  Start; Tarou ejecuta las ocho mecánicas con la animación correcta y el
  espejo correcto; la cámara lo sigue y se detiene en los extremos; el HUD de
  depuración muestra los valores en vivo; Start vuelve al título. Sin glitches
  gráficos, a 60 FPS.
- **Restricción de estilo del proyecto:** cada línea de código lleva su
  comentario en español; el texto en pantalla del juego va en inglés. El HUD
  de depuración es la única excepción: usa los nombres de los estados tal como
  están en el código, para que lo que se ve coincida con lo que se lee.

## 2. Decisiones tomadas (sesión del 2026-10-16)

| # | Decisión | Elegido | Motivo |
|---|---|---|---|
| 1 | Protagonista y edad | **Tarou adulto** | Pedido del autor; el arte de Pixellab existe para el adulto |
| 2 | Mecánicas | caminar, correr, saltar, agacharse y caminar agachado, envainar/desenvainar, ataque básico, dormir y despertar | Lista del autor. Sin colisión por tiles (exige tileset y Tiled: otra sesión) |
| 3 | Tamaño y técnica del sprite | **64×64 como metasprite dinámico del SDK**: 4 piezas de 32×32, `gfx4snes -s 32 -T -X 64 -Y 64`, `oamMetaDrawDyn32` | Con ~55 cuadros no cabe nada estático (límite 512 tiles = 8 cuadros). Es el motor de LikeMario y sirve para enemigos |
| 4 | Tamaño de ROM | **16 bancos (512 KB)** | ~115 KB libres no bastan con margen: los cuadros ocupan ~110 KB |
| 5 | Mando | B salta, Y corre, A ataca, R envaina/desenvaina, Abajo agacha, SELECT HUD, START título; X y L libres | Ver [§3](#3-mando) |
| 6 | Salto | altura variable, control aéreo, sin doble salto | Expresivo y barato de programar |
| 7 | Correr | Y mantenido, sin derrape | Convención Nintendo |
| 8 | Agacharse | puede saltar agachado; no ataca agachado | Simplicidad |
| 9 | Espada | arranca envainada; R alterna con animación; A ataca solo con espada fuera y en suelo; sin ataque aéreo; la caja de golpe se calcula y se muestra en el HUD | Base para enemigos en Etapa 3 |
| 10 | Dormir | por inactividad (~10 s), cualquier botón despierta | «Animación de espera» clásica |
| 11 | Modelo | **Estado + Espada + Dirección** (tres variables) | Sin estados duplicados «con espada» |
| 12 | HUD de depuración | texto en BG3 alternado con SELECT, apagado al inicio | Verificar la física sin el depurador; enseñar el punto fijo |
| 13 | Organización de `src/` | subcarpetas por rol: `sistema/`, `pantallas/`, `juego/` y stubs | `snes_rules` compila `src/*/*.c` (verificado) |
| 14 | «Juego completo» | MVP + **esqueleto de stubs** por subsistema futuro | Los stubs no contienen lógica, solo su contrato previsto |
| 15 | Nombre | **Tarou** en todo (título, `ROMTITLE`, archivos, variables, documentos) | Pedido del autor |
| 16 | Arte | personaje de **Pixellab** del autor (64×64, todas las animaciones); mientras no esté descargado, **marcadores numerados** con los mismos nombres | El arte real entra sin tocar código |
| 17 | Sprites antiguos | el lobo samurái (`tarouh_lobo`) pasa a `lobo_samurai` y será un **enemigo**; Bárbara conserva su sprite hasta la Revisión 2 | Pedido del autor |
| 18 | Intro y título | se cambian **ahora** para mostrar al Tarou nuevo (4 cuadros de reposo, carga estática) junto a Bárbara | Pedido del autor |
| 19 | Lógica pura separada del hardware, con pruebas en PC | `tarou.c`, `camara.c`, `nivel.c` no tocan la PPU y se compilan también con `gcc` | Permite TDD y enseña «la física no sabe de la PPU». Decisión del plan, a revisar |
| 20 | Escenario | el del diseño v1: nivel 1 de 512 px, suelo plano, dos capas con parallax, cielo HDMA | Sin cambios |

## 3. Mando

| Botón | Acción | Lectura |
|---|---|---|
| Izquierda / Derecha | caminar; en el aire, control aéreo; agachado, caminar agachado | mantenido |
| Abajo | agacharse (en suelo) | mantenido |
| Y | correr (velocidad máxima mayor) | mantenido |
| B | saltar; mantenerlo alarga el salto | pulsado + mantenido |
| A | atacar con la espada (solo con espada fuera y en suelo) | pulsado |
| R | envainar / desenvainar | pulsado |
| SELECT | alternar el HUD de depuración | pulsado |
| START | volver al título con fundido | pulsado |
| X, L | libres | — |

«Pulsado» = `padsDown(0)` (solo el frame en que baja); «mantenido» =
`padsCurrent(0)`. Cualquier botón o dirección cuenta como «actividad» para el
contador de sueño.

## 4. Estados del protagonista

Trece estados, uno activo a la vez. Cada estado tiene una animación del mismo
nombre (relación 1:1, [§7](#7-sprite-y-animación)).

| Estado | Suelo | Mueve en x | Gravedad | Sale cuando |
|---|---|---|---|---|
| `PARADO` | sí | no (frena) | no | dirección → `CAMINANDO`/`CORRIENDO`; Abajo → `AGACHADO`; B → salto; R → `DESENVAINANDO`/`ENVAINANDO`; A con espada fuera → `ATACANDO`; `FRAMES_PARA_DORMIR` sin actividad → `ECHANDOSE` |
| `CAMINANDO` | sí | sí, hasta `VEL_CAMINAR_MAX` | no | sin dirección y `vx == 0` → `PARADO`; Y → `CORRIENDO`; Abajo → `AGACHADO` (vx = 0); B → salto; R → `DESENVAINANDO`/`ENVAINANDO` (vx = 0); A con espada fuera → `ATACANDO` (vx = 0) |
| `CORRIENDO` | sí | sí, hasta `VEL_CORRER_MAX` | no | igual que `CAMINANDO`; sin Y → `CAMINANDO` (frena hasta `VEL_CAMINAR_MAX`) |
| `SALTANDO` | no | control aéreo (`ACEL_AIRE`) | sí | `vy >= 0` → `CAYENDO`; soltar B con `vy < VEL_SALTO_CORTE` recorta `vy` a `VEL_SALTO_CORTE` |
| `CAYENDO` | no | control aéreo | sí, hasta `VEL_TERMINAL` | pies en el suelo → aterriza: `AGACHADO` si Abajo; si no, `CORRIENDO`/`CAMINANDO` si `vx != 0`; si no, `PARADO` |
| `AGACHADO` | sí | no (frena) | no | dirección → `AGACHADO_CAMINANDO`; soltar Abajo → `PARADO`; B → salto (se levanta) |
| `AGACHADO_CAMINANDO` | sí | sí, hasta `VEL_AGACHADO_MAX` | no | sin dirección y `vx == 0` → `AGACHADO`; soltar Abajo → `CAMINANDO`; B → salto |
| `DESENVAINANDO` | sí | no | no | fin de animación → `espada = FUERA`, `PARADO`. Ignora el mando |
| `ENVAINANDO` | sí | no | no | fin de animación → `espada = ENVAINADA`, `PARADO`. Ignora el mando |
| `ATACANDO` | sí | no (vx = 0) | no | fin de animación → `PARADO`. Ignora el mando. La caja de golpe está activa todos sus cuadros (a afinar) |
| `ECHANDOSE` | sí | no | no | fin de animación → `DORMIDO`; cualquier actividad → `DESPERTANDO` |
| `DORMIDO` | sí | no | no | cualquier actividad → `DESPERTANDO` (animación en bucle) |
| `DESPERTANDO` | sí | no | no | fin de animación → `PARADO`. Ignora el mando |

Reglas transversales:

- **Salto** (desde cualquier estado de suelo que lo permita): `vy = VEL_SALTO`,
  `en_suelo = 0`, estado `SALTANDO`; conserva `vx` (correr alarga el salto).
- **Prioridad el mismo frame:** START > SELECT > R > A > B > Abajo > dirección.
  Dos pulsaciones simultáneas resuelven por esta lista.
- **Dirección:** la fija la última dirección pulsada, en suelo o en el aire.
  No cambia mientras un estado ignora el mando.
- **Contador de sueño:** `frames_quieto` sube solo en `PARADO`; cualquier
  actividad lo pone a 0.

## 5. Movimiento y física

- **Coordenadas del mundo:** Tarou vive en píxeles del mundo, `x` de 0 a
  `ANCHO_NIVEL − 64 = 448` (la celda mide 64). El píxel que usan la OAM y el
  scroll es siempre la **parte entera** (`x_fp >> 4`): sprite y fondo
  comparten el mismo entero y nunca se desfasan.
- **Punto fijo 12.4 en `s16`:** 12 bits de entero y 4 de fracción (1/16 px).
  **Las posiciones nunca son negativas** (se recortan antes de convertir), así
  el desplazamiento `>> 4` es seguro en `tcc816`. Solo las velocidades pueden
  ser negativas y nunca se convierten a píxel.
- **Constantes iniciales (a afinar en Mesen con el HUD):**

| Constante | Valor (fp) | En píxeles | Efecto |
|---|---|---|---|
| `VEL_CAMINAR_MAX` | 16 | 1,0 px/frame | 60 px/s |
| `VEL_CORRER_MAX` | 32 | 2,0 px/frame | cruza el nivel en ~4 s |
| `VEL_AGACHADO_MAX` | 8 | 0,5 px/frame | |
| `ACEL_SUELO` | 2 | | 8 frames hasta caminar, 16 hasta correr |
| `FRENADO_SUELO` | 2 | | 8 frames hasta parar desde caminar |
| `ACEL_AIRE` | 1 | | control aéreo suave |
| `GRAVEDAD` | 4 | 0,25 px/frame² | |
| `VEL_SALTO` | −96 | −6,0 px/frame | apogeo a los 24 frames, ~72 px de altura |
| `VEL_SALTO_CORTE` | −32 | −2,0 px/frame | salto corto ≈ 8 px tras soltar |
| `VEL_TERMINAL` | 96 | 6,0 px/frame | |
| `FRAMES_PARA_DORMIR` | 600 | | 10 s |

- **Cada frame, en este orden:** leer mando → transiciones de estado →
  aceleración/frenado en x → gravedad en y → integrar (`x += vx`, `y += vy`)
  → recortar x a [0, 448] → aterrizaje (si `y_pix + 64 >= suelo`: `y = suelo −
  64`, `vy = 0`, `en_suelo = 1`) → avanzar animación.
- **Suelo:** `nivel_suelo_en(x_pix)` devuelve hoy la constante `SUELO_Y = 208`
  (a medir sobre el fondo). La última fila del cuadro es la planta de las
  patas: `y_celda = suelo − 64`.
- **Cajas** (coordenadas relativas a la celda de 64×64, a afinar sobre el
  arte):
  - cuerpo de pie: x 20..44, y 8..64; agachado: y 32..64;
  - golpe (solo en `ATACANDO`): 28×16 px delante del cuerpo, y 24..40, x
    44..72 mirando a la derecha; en espejo mirando a la izquierda.

## 6. Cámara

- `camara = x_pix + 32 − 128`, recortada a 0..`ANCHO_NIVEL − ANCHO_PANTALLA`
  (0..256). Tarou se mueve hasta el centro, la cámara lo sigue y, al final,
  llega al borde. Sin suavizado.
- `x_pantalla = x_pix − camara` (nunca negativo); `y_pantalla = y_pix` (no hay
  scroll vertical).
- Capa cercana: scroll = `camara`. Capa lejana: scroll = `camara >> 1`.

## 7. Sprite y animación

- **Motor:** sprites dinámicos del SDK. `oamInitDynamicSprite(0x0000, 0x1000,
  0, 0, OBJ_SIZE16_L32)`: piezas grandes de 32×32 desde VRAM 0x0000. Tarou usa
  las entradas 0..3 de la OAM, paleta de sprites 0 (CGRAM 128), prioridad 3.
- **Por frame:** `oamMetaDrawDyn32(0, x_pantalla, y_pantalla, tabla, tiles)`
  en la actualización; `oamInitDynamicSpriteEndFrame()` al final de la
  actualización; `oamVramQueueUpdate()` justo después de `WaitForVBlank()`.
  Para esto la secuencia gana una tercera función por pantalla:
  `pantalla_X_vblank()`.
- **Espejo:** la tabla metasprite de cada cuadro viene de `gfx4snes -T`
  mirando a la derecha. Mirando a la izquierda se construye en RAM una copia
  de 5 entradas con `dx` intercambiado (0 ↔ 32) y el bit `hflip` (0x40) en los
  atributos. Sin variantes `-L` del conversor (menos incógnitas).
- **Animaciones (1:1 con los estados):** `reposo`, `caminar`, `correr`,
  `saltar`, `caer`, `agachado`, `agachado_caminar`, `desenvainar`, `envainar`,
  `atacar`, `echarse`, `dormido`, `despertar`. Cada una: cantidad de cuadros
  (la fija el arte y la genera el pipeline en `tarou_anim.h`), frames por
  cuadro y si hace bucle.

| Animación | Frames/cuadro (a afinar) | Bucle |
|---|---|---|
| reposo | 12 | sí |
| caminar | 6 | sí |
| correr | 4 | sí |
| saltar, caer | 6 | no (se queda en el último cuadro) |
| agachado | 12 | sí |
| agachado_caminar | 8 | sí |
| desenvainar, envainar | 5 | no |
| atacar | 4 | no |
| echarse, despertar | 8 | no |
| dormido | 20 | sí |

- **Presupuesto:** cada cuadro de 64×64 son 2 KB en ROM; con ~55 cuadros,
  ~110 KB. Cada animación va en su propia sección de `data.asm` y **no puede
  pasar de 15 cuadros** (30 KB < 32 KB por banco); el pipeline lo comprueba.

## 8. Fondo

- **Modo:** Mode 1 con `BG3_MODE1_PRIORITY_HIGH`. BG0 cercana, BG1 lejana, BG2
  texto del HUD (fuente 2bpp del título), apagado salvo con el HUD.
- **Capa cercana (BG0):** los arbustos del título, `cerca_*`, 512 px en
  `SC_64x32`, paleta 1, en `VRAM_FONDO_A` / `VRAM_MAPA_A`. Scroll 1:1.
- **Capa lejana (BG1):** `res/nivel1/fondo_lejos.png`, derivada del título por
  `scripts/preparar_fondo_nivel.py` (borra el logo horneado y duplica en espejo
  hasta 512 px), paleta 2, `SC_64x32` en `VRAM_FONDO_B` / `VRAM_MAPA_B`
  (0x7400–0x7BFF). Scroll a la mitad.
- **Cielo:** el degradado HDMA del título (`hdmaGradcieloList`, canal 6), que
  se corta al salir con `setModeHdmaReset(0)`.

## 9. HUD de depuración

Capa BG2 (BG3 del hardware) con la fuente 2bpp. SELECT la enciende y apaga
(`bgSetEnable(2)` / `bgSetDisable(2)` + limpiar). Arranca apagada. Filas:

```
X 123  VX -16        (píxel entero y velocidad en fp)
Y 144  VY 0
EST CAMINANDO
ESP FUERA  DIR DER
CAM 0
GOLPE 167 168 28 16  (x, y, ancho, alto en píxeles del mundo; «-» si no hay)
```

## 10. Pipeline de assets del protagonista

```
res/descargas/graficos/pixellab/tarou/      ← el autor descarga aquí (ignorado por git)
    README.md                               ← qué exportar y cómo nombrarlo (versionado)
    reposo.png | reposo/01.png... | reposo.gif   (una entrada por animación)
          │  python scripts/preparar_personaje.py tarou
          ▼
res/personajes/tarou/                       ← versionado (como el resto de res/)
    tarou_reposo.png ... tarou_despertar.png   tiras indexadas, 64×64 por cuadro,
                                               UNA paleta compartida (15 + transparente)
    tarou_anim.h                               #define TAROU_REPOSO_CUADROS 4 ...
          │  make  (gfx4snes -s 32 -o 16 -u 16 -p -T -X 64 -Y 64 -P 3)
          ▼
    tarou_reposo.pic / .pal / tarou_reposo_meta.inc   (ignorados; data.asm y tarou_sprite.c los usan)
```

- Entrada aceptada por animación: tira horizontal PNG, carpeta con cuadros
  numerados, o GIF. Si falta una animación, el script genera un **marcador**:
  cuadros de 64×64 con el nombre y el número del cuadro.
- Alineación: una sola caja envolvente común a **todas** las animaciones, pies
  al borde inferior. Así no hay saltos entre animaciones.
- Paleta: se cuantizan juntos todos los cuadros de todas las animaciones a 15
  colores; el índice 0 es transparente. Los `.pal` que genere gfx4snes deben
  ser idénticos; se carga solo el de `reposo`.
- Lo mismo vale para `barbara` en la Revisión 2 (misma carpeta, mismo script).

## 11. Arquitectura

```
src/
├── sistema/        main.c · secuencia.c/.h · comun.c/.h · pantallas.h · tipos.h · puntofijo.h
├── pantallas/      pantalla_nintendo.c · _publisher.c · _devs.c · _dedicatoria.c
│                   · _intro.c · _titulo.c · _nivel1.c · personajes.c/.h
├── juego/          mando.c/.h · nivel.c/.h · camara.c/.h · animaciones.c/.h
│                   · tarou.c/.h (lógica pura) · tarou_sprite.c/.h (OAM) · hud_debug.c/.h
│                   · barbara.c/.h [stub R2] · cambio_protagonista.c/.h [stub Etapa 4]
├── enemigos/       enemigos.c/.h [stub Etapa 3]
├── objetos/        objetos.c/.h [stub Etapa 3]        (ítems)
├── hud/            hud.c/.h [stub Etapa 3]            (vida, ítems; distinto de hud_debug)
├── audio/          audio.c/.h [stub, sesión de audio]
└── niveles/        niveles.c/.h [stub Etapa 2/3]      (carga de niveles por tiles)
tests/              pruebas en PC con gcc: minitest.h · pruebas.c · prueba_*.c · Makefile
```

Flujo del nivel, por frame:

```
main.c ─ secuencia_actualizar() ─▶ pantalla_nivel1_actualizar()
                                      mando_leer(&mando)                [juego/mando.c]
                                      START → salir(PANTALLA_TITULO)
                                      SELECT → hud_debug_alternar()
                                      tarou_actualizar(&tarou, &mando, suelo)   [lógica pura]
                                      camara = camara_para(x_pix)
                                      bgSetScroll(0, camara) · bgSetScroll(1, camara>>1)
                                      tarou_sprite_dibujar(&tarou, camara)      [OAM]
                                      hud_debug_dibujar(&tarou, camara)
                                      oamInitDynamicSpriteEndFrame()
       ─ WaitForVBlank()
       ─ secuencia_vblank() ───────▶ pantalla_nivel1_vblank(): oamVramQueueUpdate()
```

- **Lógica pura** (`tarou.c`, `camara.c`, `nivel.c`, `animaciones.c`): sin
  `snes.h`. Incluyen `sistema/tipos.h`, que en la SNES toma `u8/u16/s16` de
  `snes.h` y en el PC (`-DPRUEBAS_PC`) de `stdint.h`. Se compilan en la ROM
  con `tcc816` y en `tests/` con `gcc`.
- **Capa de hardware** (`mando.c`, `tarou_sprite.c`, `hud_debug.c`,
  `pantalla_nivel1.c`): traduce entre la lógica y la PPU/mando.
- **Intro y título:** `personajes.c` muestra a Tarou (4 cuadros de reposo,
  carga estática de 8 KB en VRAM 0x0000, dibujado con 4 `oamSet` de 32×32 a
  partir de la misma tabla metasprite) y a Bárbara (sprite estático de 64×64
  como hasta ahora, tiles a continuación). Tamaño `OBJ_SIZE32_L64`.

## 12. Cabecera estándar de cada archivo fuente

Todo `.c`, `.h` y `.asm` empieza con este bloque (en `.asm`, con `;`):

```c
/*---------------------------------------------------------------------------------
    nombre.c — Una línea: qué es este archivo.

    PROPÓSITO
      Dos o tres frases: qué problema resuelve y cómo.

    SE CONECTA CON
      Incluye:  a.h (para X), b.h (para Y)
      Lo usan:  c.c (llama a f), d.c (lee g)

    HARDWARE QUE TOCA
      PPU: registros de scroll (bgSetScroll) · OAM (oamSet) · ninguno

    VARIABLES (globales y estáticas de este archivo)
      nombre      tipo   rango / unidad          para qué sirve
      x_fp        s16    0..7168, punto fijo 12.4 posición en el mundo

    → glosario: Término1, Término2
---------------------------------------------------------------------------------*/
```

Los stubs llevan además la línea `PENDIENTE: Etapa N — qué hará`.

## 13. Verificación

No hay suite de tests en la SNES. Tres niveles:

1. **Pruebas en PC** (`make -C tests && tests/pruebas.exe`): la lógica pura
   con los valores de §5 (aceleración, salto, corte, aterrizaje, cámara,
   transiciones, animación). Todas en verde antes de cada commit.
2. **Compilación de la ROM** desde PowerShell (comando de `CLAUDE.md`), con
   `make clean` tras tocar cabeceras. Sin errores ni avisos nuevos; el `.log`
   dice 16 bancos.
3. **Mesen**, con capturas de `scripts/capturas_mesen.lua` (si Mesen bloquea
   la escritura, activar el acceso a E/S de scripts en su configuración):
   1. Start en el título entra al nivel con fundido; el HUD arranca apagado.
   2. Caminar y correr a ambos lados; espejo correcto; la cámara se detiene en
      0 y 256; Tarou llega a los dos bordes.
   3. Salto largo (B mantenido) y corto (B soltado); control aéreo.
   4. Abajo agacha; Abajo + dirección camina agachado; B agachado salta.
   5. R desenvaina y envaina con animación; A solo ataca con espada fuera;
      el HUD muestra la caja de golpe durante el ataque.
   6. 10 s quieto: se echa, duerme; un botón lo despierta.
   7. Las dos capas se desplazan a distinta velocidad sin costuras.
   8. START vuelve al título; intro y título muestran al Tarou nuevo.

## 14. Fuera de alcance

Colisión con el escenario, desniveles, tileset y Tiled · enemigos y daño ·
ítems · Bárbara jugable y cambio de protagonista (Revisión 2 / Etapa 4) ·
edades cachorro y vieja · audio · HUD del juego · derrape · ataque aéreo y
agachado · variantes de animación con espada fuera · cámara suavizada.

## 15. Riesgos y puntos a verificar

- **Metasprite estático en intro y título:** la numeración de bloques de
  `gfx4snes -s 32` (filas de 128 px, bloque `n` → tile `(n/4)*64 + (n%4)*4`)
  se comprueba con el marcador numerado antes de fiarse de la fórmula.
- **Nombres de las tablas `_meta.inc`:** se leen del archivo generado antes de
  escribir `tarou_sprite.c`.
- **`consoleDrawText` con `%d` negativos:** se comprueba en el HUD.
- **Altura del suelo y cajas:** se miden sobre el arte real cuando llegue.
- **Arte de Pixellab:** puede no venir «en el sitio» ni con 15 colores; el
  pipeline cuantiza y alinea, pero el resultado se revisa a ojo.
- **`tcc816`:** evitar `int`, `long`, `float`, `bool`, punteros a función y
  aritmética de 32 bits en la lógica pura, que también compila en gcc.
- **Cambios de `pantallas.h` y `tipos.h`:** `make clean` antes de `make`.
