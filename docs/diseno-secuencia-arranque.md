# Diseño — Demo técnica: secuencia de arranque

**Proyecto:** _Tarouh: A Dog's Journey_ · **Etapa del plan:** 1.5 · **Fecha de
diseño:** 2026-09-23 · **Estado:** aprobado por entrevista (35 decisiones), en
implementación.

Este documento es la especificación de trabajo de la secuencia de arranque del
juego: todo lo que pasa desde que se enciende la consola hasta que el jugador
presiona Start en la pantalla de título. Es un **documento vivo**: cambia cada
vez que se reemplaza un asset provisional o se ajusta un tiempo tras verlo en
Mesen. Las decisiones de fondo (por qué se eligió cada cosa) están en la
sección [Decisiones](#decisiones-tomadas-en-la-entrevista); el resumen para la
hoja de ruta está en `docs/plan-desarrollo.md`, Etapa 1.5.

---

## 1. Propósito y alcance

- **Qué es:** el código **definitivo** de la secuencia de arranque, no un
  experimento. Sirve además como demo técnica para medir alcances: qué tan
  lejos se puede llevar el hardware (Mode 7, HDMA, parallax, audio SPC) antes
  de tocar gameplay.
- **Qué no es:** no incluye gameplay. Al presionar Start no arranca ningún
  nivel; se muestra una pantalla "LEVEL 1 — COMING SOON" que vuelve al título.
- **Para quién:** proyecto personal, sin venta ni distribución. Por eso se usa
  material original con copyright de la época (logo de Nintendo, logos de
  publisher/desarrollador de ejemplo) en uso privado. Cada logo vive aislado
  como un asset en `res/` para poder reemplazarlo sin tocar código si algún día
  cambia la premisa.
- **Criterio de éxito:** una ROM `.sfc` que, al cargarse en Mesen, recorre las
  seis pantallas en orden sin intervención, llega al título, entra en modo
  attract a los 15 s, y responde a Start con la transición a "coming soon" y
  de vuelta. Sin glitches gráficos ni de audio, a 60 FPS.

## 2. Estética y referencias

- **Referencia visual:** _Secret of Mana_ — pixel art detallado, paletas ricas,
  a medio camino entre plataformas y RPG. El arte debe "exprimir" las
  capacidades de la SNES: varias capas, degradados por línea, rotación/zoom.
- **Textos en pantalla:** en **inglés** (título, créditos, intro, "PRESS
  START"). El código y sus comentarios siguen en español.
- **Fuente de texto:** provisional, la fuente embebida de PVSnesLib
  (`consoleInitDefaultText`). Se reemplaza después por una fuente de época
  (BMP 8 bits, regla `-s 8 -o 2 -u 16 -e 1 -p -t bmp -m -R`) sin tocar lógica.
- **Vídeo:** NTSC 60 Hz, 256×224, **Mode 1** como base (BG1/BG2 de 16 colores,
  BG3 de 4 colores). **Mode 7** solo en la pantalla de desarrolladores.

## 3. Secuencia de pantallas

Orden final (convención de época: Nintendo → publisher → desarrollador):

```
 [1] NINTENDO ──► [2] PUBLISHER ──► [3] DEVELOPERS ──► [4] DEDICATION
                                                              │
                                                              ▼
       ┌──────────────────────────────────────────── [5] INTRO (3 escenas)
       │                                                      │
       │  attract: 15 s sin input                             ▼ mosaico
       └───────────────────────────────────────────── [6] TITLE ◄──────┐
                                                              │ Start   │ Start
                                                              ▼         │
                                                     [7] COMING SOON ───┘
```

Ninguna pantalla se puede saltar por ahora. Cada pantalla tiene una constante
`PERMITE_SALTO` fijada en 0; ponerla en 1 habilita saltar con Start (previsto
para el desarrollo posterior).

### Tiempos acordados

| Pantalla | Duración | Entrada | Salida |
| --- | --- | --- | --- |
| 1 Nintendo | 3 s | fade in 0,5 s | fade out 0,5 s |
| 2 Publisher | 3 s | fade in | fade out |
| 3 Developers | 4 s (2 s zoom Mode 7 + 2 s quieto) | desde negro | fade out |
| 4 Dedication | 7 s | fade in lento (1 s) | fade out lento (1 s) |
| 5 Intro | 3 × 8 s | fade in | fade entre escenas; **mosaico** hacia título |
| 6 Title | indefinida; attract a los 15 s sin input | mosaico in | fade out |
| 7 Coming soon | hasta Start | fade in | fade out → título |
| "PRESS START" | parpadeo 0,5 s encendido / 0,5 s apagado | | |

Todos los tiempos se expresan en **frames** en el código (60 frames = 1 s en
NTSC) y se ajustan a ojo en Mesen.

### Pantalla 1 — Nintendo

- **Contenido:** logo "Nintendo" rojo centrado sobre fondo negro y, debajo, el
  texto "LICENSED BY NINTENDO". Muda (como casi todas las de la época).
- **Técnica:** Mode 1. BG1 = imagen del logo (16 colores, 4bpp). Texto con la
  fuente de consola en BG3 (o dentro de la misma imagen si se prefiere una sola
  pieza). `setFadeEffectEx(FADE_IN, 30)` → espera → `setFadeEffectEx(FADE_OUT, 30)`.
- **Asset:** `res/nintendo/logo_nintendo.png` (256×224, indexado, ≤16 colores).
  Lo descarga el usuario; se adapta con `scripts/preparar_imagen.py`.

### Pantalla 2 — Publisher (provisional: Capcom)

- **Contenido:** logo de Capcom con jingle, tomado del ejemplo del SDK
  `tools/pvsneslib/snes-examples/thirdparty/logo/snes-logo-capcom/`.
- **Técnica:** Mode 1, `setMode(BG_MODE1, BG3_MODE1_PRIORITY_HIGH)`, fade,
  `spcLoad(MOD_LOGO_PUBLISHER)` + `spcPlay(0)`.
- **Assets:** `res/publisher/logo_publisher.bmp` (96×32 en el ejemplo) y
  `res/musica/jingle_publisher.it`. Reemplazables.

### Pantalla 3 — Developers (Mode 7)

- **Contenido:** una imagen de 256 colores con el logo provisional del estudio
  (estilo Konami/época) arriba y, debajo, "ALEJANDRO MORA" con los roles
  "PROGRAMMING · ART · MUSIC". La imagen **entra desde lejos girando** (zoom +
  rotación Mode 7) durante 2 s hasta encajar a escala 1:1, y queda quieta 2 s.
- **Técnica:** Mode 7 (`bgInitMapTileSet7`, `setMode7`, `setMode7Rot` /
  escala por frame). Mode 7 ocupa toda la pantalla y solo tiene BG1, por eso
  el texto va **horneado en la misma imagen**. Mejora futura: escribir el
  nombre con sprites (OAM funciona en Mode 7) para que no gire.
- **Assets:** `res/devs/logo_devs.png` (256×224 u otro tamaño potencia de 2,
  indexado 256 colores, regla `-p -m -M 7`) y `res/musica/jingle_devs.it`.

### Pantalla 4 — Dedication

- **Contenido:** texto centrado sobre negro que aparece letra por letra
  (máquina de escribir), con música suave. Texto **pendiente**: el usuario lo
  entrega después; mientras tanto el marcador es `"For ..."`.
- **Técnica:** Mode 1, solo BG3 con la fuente de consola. Fade in y fade out
  lentos (60 frames). Rutina común `texto_maquina_escribir()` en `comun.c`.
- **Assets:** ninguno gráfico; `res/musica/dedicatoria.it`.

### Pantalla 5 — Intro (3 escenas)

- **Contenido:** tres escenas, cada una con un fondo distinto y un párrafo
  corto en inglés que aparece con efecto máquina de escribir en una franja
  inferior. Redacta Claude un marcador (presentación de Tarouh y Bárbara → el
  hecho que dispara el viaje → el camino por delante); el usuario lo corrige.
  El guion definitivo no está escrito y estos textos no lo comprometen.
- **Sprite:** en la tercera escena aparece **Tarouh cachorro** (sprite OAM,
  16×16 o 32×32, 16 colores, animación de 2 a 4 cuadros). Sin Bárbara en esta
  demo (premisa del plan).
- **Técnica:** Mode 1. BG1 = fondo de la escena (16 colores). BG3 = texto con
  prioridad alta (`BG3_MODE1_PRIORITY_HIGH`) sobre una franja oscura. Fade
  entre escenas. Salida hacia el título con **mosaico**
  (`setMosaicEffect(MOSAIC_IN, MOSAIC_BG1)`).
- **Assets:** `res/intro/escena_1.png`, `escena_2.png`, `escena_3.png`
  (256×224, indexado, ≤16 colores cada uno), `res/intro/tarouh_cachorro.png`
  (spritesheet, indexado 16 colores), `res/musica/intro.it`. Si solo se
  consigue un fondo, se reutiliza en las tres escenas sin tocar código.

### Pantalla 6 — Title

- **Contenido:** logo del título "Tarouh: A Dog's Journey", fondo con
  **parallax** de dos capas, **cielo con degradado HDMA**, Tarouh cachorro
  animado en reposo, "PRESS START" parpadeando y la línea "© 2026 <ESTUDIO>"
  al pie. Música en bucle.
- **Técnica:**
  - BG1 = primer plano (scroll rápido, `bgSetScroll` cada frame).
  - BG2 = fondo lejano (scroll lento). El parallax se hace por software con
    dos velocidades; **no** con `setParallaxScrolling`, porque esa función usa
    el canal HDMA 3 y chocaría con el degradado.
  - Degradado: `setModeHdmaGradient()` sobre el color de fondo (canal 3).
  - BG3 = logo del título (4 colores: 3 + transparente) + textos. Mejora
    futura: logo como sprites de 32×32 (16 colores).
  - Tarouh = OAM, misma spritesheet que la intro.
  - Attract: contador de frames sin input; a los 900 frames (15 s) →
    fade out → Intro.
  - Start (`padsDown(0) & KEY_START`) → fade out → Coming soon.
- **Assets:** `res/titulo/logo_titulo.png` (generado por
  `scripts/generar_logo_titulo.py` a partir de una fuente libre; reemplazable),
  `res/titulo/fondo_cerca.png`, `res/titulo/fondo_lejos.png` (256×224 o
  512×224 para scroll, ≤16 colores cada uno), `res/musica/titulo.it`.

### Pantalla 7 — Coming soon

- **Contenido:** "LEVEL 1 — COMING SOON" centrado sobre negro, y debajo
  "PRESS START TO RETURN".
- **Técnica:** Mode 1, solo texto. Es el **enchufe** para el gameplay: la
  función `iniciar_juego()` que hoy muestra este texto será reemplazada por la
  carga del Nivel 1 en la Etapa 2/3.

## 4. Arquitectura del código

```
src/
  main.c                  inicialización de consola y audio; bucle principal
  secuencia.c / .h        máquina de estados: enum Pantalla + switch; contador
                          de frames por pantalla; cambio de pantalla
  pantallas.h             prototipos comunes: cada pantalla expone
                          pantalla_X_iniciar() y pantalla_X_actualizar()
  pantalla_nintendo.c
  pantalla_publisher.c
  pantalla_devs.c
  pantalla_dedicatoria.c
  pantalla_intro.c
  pantalla_titulo.c
  pantalla_proximamente.c
  comun.c / .h            utilidades: esperar_frames(), fade_in/out(),
                          texto_maquina_escribir(), parpadeo()
data.asm                  secciones ".rodataN" superfree, una por pantalla,
                          con .include de los *_data.as que genera gfx4snes
                          y .incbin de los .brr
res/                      assets (ver §5)
scripts/                  herramientas Python de preparación de assets
docs/glosario-snes.md     glosario de conceptos SNES referenciado por comentarios
```

**Máquina de estados.** Un `enum` con un valor por pantalla y una variable
`pantalla_actual`. Cada frame el bucle principal llama a
`secuencia_actualizar()`, que ejecuta el `switch` sobre `pantalla_actual` y
llama a la función `actualizar()` de la pantalla activa. Cuando una pantalla
termina, llama a `secuencia_cambiar(SIGUIENTE)`, que ejecuta el `iniciar()` de
la nueva (carga de tiles, paletas, mapas, música) y pone el contador de frames
en cero. Se eligió este esquema, y no `gamestates.h` del SDK, porque un
`enum` + `switch` se explica línea por línea sin punteros a función ni macros.

**Bucle principal.** Siempre `WaitForVBlank()` una vez por vuelta: el mando
solo se actualiza en VBlank y `spcProcess()` debe llamarse cada frame.

**Convención de explicación.** Cada línea de código lleva un comentario en
español que explica qué hace, qué variables usa y qué sintaxis de C aparece.
Los conceptos de hardware (VRAM, CGRAM, OAM, VBlank, DMA, paleta, tile, banco,
HDMA, Mode 7) se explican **una sola vez** en `docs/glosario-snes.md`; los
comentarios lo referencian con `→ glosario: VRAM`.

## 5. Assets y pipeline

### Carpetas

```
res/
  descargas/            ← AQUÍ deposita el usuario lo que baja (no se compila)
    graficos/           PNG/BMP tal cual vienen (Spriters Resource, OpenGameArt, itch)
    audio/              .it / .xm / .mod / .wav tal cual vienen
    README.md           qué formatos sirven y cómo se convierten
  nintendo/  publisher/  devs/  dedicatoria/  intro/  titulo/
                        ← assets ya preparados (indexados) que el Makefile compila
  musica/               .it de música y jingles
  sfx/                  .wav mono cortos → .brr
```

### Formatos que acepta el hardware/toolchain

| Tipo | Formato de entrada | Herramienta | Límite |
| --- | --- | --- | --- |
| Fondo / logo Mode 1 | PNG o BMP indexado 256 col., ≤16 colores usados | `gfx4snes -s 8 -o 16 -u 16 -p -m` | 256×224 (o 512 de ancho para scroll) |
| Imagen Mode 7 | PNG/BMP indexado 256 colores | `gfx4snes -p -m -M 7` | tiles de 8×8, hasta 256 tiles únicos |
| Spritesheet | PNG/BMP indexado, ≤16 colores | `gfx4snes -s 16 -o 16 -u 16 -p` (o `-s 32`) | ancho múltiplo del tamaño de sprite |
| Fuente | BMP 8 bits, 768×8 | `gfx4snes -s 8 -o 2 -u 16 -e 1 -p -t bmp -m -R` | 96 caracteres |
| Música | **.it** (Impulse Tracker) | `smconv -s -o res/musica/soundbank -V -b 5` | 8 canales; samples ≤ 58 KB tras BRR; música+efectos < 60 KB |
| Efecto | .wav mono 16 bits, corto | `snesbrr -e` (regla implícita) | región de sonido limitada en la RAM del SPC |

**No sirven:** `.spc` (rips de SNES), `.mp3`, `.ogg`, `.wav` largos como
música. Los `.xm`/`.mod`/`.s3m` se convierten a `.it` con OpenMPT (gratis).

### Scripts (Python 3 + Pillow)

- `scripts/preparar_imagen.py <entrada> <salida> [--colores 16|256] [--ancho W --alto H]`
  Recorta/centra al tamaño pedido, reduce a la cantidad de colores indicada y
  guarda PNG indexado listo para `gfx4snes`. Ejecutar desde la raíz del repo.
- `scripts/generar_logo_titulo.py`: renderiza "TAROUH" y "A Dog's Journey" con
  una fuente libre a un PNG indexado de 4 colores para BG3
  (`res/titulo/logo_titulo.png`).

### Makefile

Antes del `include snes_rules`: `AUDIOFILES` (lista de `.it`, los de efectos
primero) y `export SOUNDBANK := res/musica/soundbank`. Después: una regla
explícita `.pic` por asset (PNG no tiene regla implícita), el objetivo
`bitmaps`, `musics: $(SOUNDBANK).obj`, y `clean` que además borre los `.brr`.

### Banking

Cada pantalla tiene su sección `".rodataN" superfree` en `data.asm`, de modo
que ninguna supere 32 KB. El soundbank va al banco 5 (y 6 si crece). Para ver
el uso: `snesromusage.exe -i ProyectoSNES.sym`. Si la ROM se llena, subir
`ROMBANKS` y `ROMSIZE` en el Makefile.

## 6. Verificación

1. `run.bat` compila y abre Mesen. La secuencia debe recorrer 1→6 sin tocar
   nada, en los tiempos de la tabla (cronometrar con el contador de frames de
   Mesen o a ojo).
2. En el título: no tocar nada 15 s → vuelve a la intro (escena 1).
3. En el título: Start → fade → "COMING SOON"; Start → fade → título con la
   música reiniciada.
4. Sin sprites basura, sin filas de tiles corruptas al cambiar de Mode 7 a
   Mode 1, sin cortes de audio.
5. `make` sin warnings nuevos; `snesromusage` muestra todas las secciones
   dentro de sus bancos.
6. Hardware real: fuera del alcance de esta etapa (queda en la Etapa 6).

## 7. Decisiones tomadas en la entrevista

| # | Decisión |
| --- | --- |
| Q1 | Código definitivo; sin gameplay; diseños de época SNES (16 bits), quizá no definitivos. |
| Q3/Q18 | Claude escribe todo el código; **cada línea explicada** con comentario; glosario único de conceptos SNES. |
| Q4 | La Etapa 0 sigue en paralelo; se adelanta el tema binario/hexadecimal. |
| Q5/Q21 | Seis pantallas en orden de época: Nintendo → publisher → developers → dedication → intro → title. |
| Q6 | No se saltan pantallas; `PERMITE_SALTO = 0` deja la opción lista. |
| Q7 | Start → fade → "LEVEL 1 — COMING SOON" → Start → título. |
| Q8 | Attract a los 15 s, vuelve a la intro. |
| Q9/Q17 | Proyecto personal sin distribución; se usa material original con copyright; logos aislados como assets reemplazables. Premisa del plan actualizada. |
| Q10/Q23 | Logo de estudio provisional estilo Konami + "ALEJANDRO MORA" y roles. |
| Q11/Q26 | Intro: 3 escenas, texto máquina de escribir (marcador redactado por Claude), Tarouh sprite en la 3.ª. |
| Q12 | Estética _Secret of Mana_. |
| Q13/Q20 | Assets descargados por el usuario a `res/descargas/`; script Python de indexado; el arte debe exprimir la SNES. |
| Q14 | Textos en pantalla en inglés; comentarios en español. |
| Q15/Q19 | Audio: aún no descargado; carpetas listas; mientras, `.it` de los ejemplos del SDK. |
| Q16 | Nueva Etapa 1.5 en el plan. |
| Q22 | Nintendo: logo + "LICENSED BY NINTENDO", muda, fade. |
| Q24 | Publisher provisional: Capcom del SDK. |
| Q25/Q33 | Dedicatoria: texto sobre negro, 7 s, música suave; texto pendiente, marcador "For ...". |
| Q27 | Logo del título generado por script desde fuente libre. |
| Q28 | Título: parallax + HDMA + Tarouh animado + PRESS START parpadeante + © + música. |
| Q29 | Efectos: fade, mosaico, degradado HDMA, parallax, Mode 7 (devs). Transparencia y ventanas para el gameplay. |
| Q30 | Tiempos según tabla de §3. |
| Q31 | Un `.c` por pantalla + `secuencia.c` con enum/switch + `main.c` mínimo. |
| Q32 | Este documento + resumen en el plan. |
| Q34 | `res/`, rama `demo-secuencia-arranque`, Mode 1 base, fuente SDK provisional, solo Tarouh cachorro, verificación en Mesen, CLAUDE.md actualizado. |
| Q35 | Implementación arranca en la misma sesión: estructura, carpetas, máquina de estados y pantalla Nintendo con marcadores. |

## 8. Pendientes que dependen del usuario

- [ ] Texto de la dedicatoria (inglés, una o dos líneas).
- [ ] Nombre provisional del estudio para el logo de devs y el "©" del título.
- [ ] Descargas en `res/descargas/graficos/`: logo Nintendo, logo de estudio,
      3 fondos de intro, spritesheet de un cachorro, 2 fondos para el título.
- [ ] Descargas en `res/descargas/audio/`: 1 música de título, 1 de intro, 1
      suave para la dedicatoria, jingles `.it` (o se usan los del SDK).
- [ ] Corregir los tres párrafos marcador de la intro.
