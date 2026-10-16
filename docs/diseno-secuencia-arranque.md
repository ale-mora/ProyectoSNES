# Diseño — Demo técnica: secuencia de arranque

**Proyecto:** _Tarou: A Dog's Journey_ · **Etapa del plan:** 1.5 · **Fecha de
diseño:** 2026-09-23 · **Estado:** aprobado por entrevista (35 decisiones), en
implementación.

Este documento es la especificación de trabajo de la secuencia de arranque del
juego: todo lo que pasa desde que se enciende la consola hasta que el jugador
presiona Start en la pantalla de título. Es un **documento vivo**: cambia cada
vez que se reemplaza un asset provisional o se ajusta un tiempo tras verlo en
Mesen. Las decisiones de fondo (por qué se eligió cada cosa) están en la
sección [Decisiones](#7-decisiones-tomadas-en-la-entrevista); el resumen para la
hoja de ruta está en `docs/plan-desarrollo.md`, Etapa 1.5.

---

## 1. Propósito y alcance

- **Qué es:** el código **definitivo** de la secuencia de arranque, no un
  experimento. Sirve además como demo técnica para medir alcances: qué tan
  lejos se puede llevar el hardware (Mode 7, HDMA, parallax, audio SPC) antes
  de tocar gameplay.
- **Qué no es:** esta demo no incluía gameplay: Start mostraba "LEVEL 1 —
  COMING SOON". Desde la Revisión 1 (2026-10-16, `docs/diseno-demo-jugable.md`)
  Start entra al nivel 1, que se documenta allí.
- **Para quién:** proyecto personal, sin venta ni distribución. Por eso se usa
  material original con copyright de la época (logo de Nintendo, logos de
  publisher/desarrollador de ejemplo) en uso privado. Cada logo vive aislado
  como un asset en `res/` para poder reemplazarlo sin tocar código si algún día
  cambia la premisa.
- **Criterio de éxito:** una ROM `.sfc` que, al cargarse en Mesen, recorre las
  seis pantallas en orden sin intervención, llega al título, entra en modo
  attract a los 15 s, y responde a Start entrando al nivel 1 (antes, "coming
  soon"), desde donde START devuelve al título. Sin glitches gráficos ni de
  audio, a 60 FPS.

## 2. Estética y referencias

- **Referencia visual:** _Secret of Mana_ — pixel art detallado, paletas ricas,
  a medio camino entre plataformas y RPG. El arte debe "exprimir" las
  capacidades de la SNES: varias capas, degradados por línea, rotación/zoom.
- **Ambientación de la demo (decisión del 2026-09-24):** **Japón** de fantasía.
  Fondos de los packs _Spring Forest_ (cerezos, torii, herrería, 4 capas) y
  _Autumn Forest_ (bosque otoñal, pozo, farolillos, 3 capas); música japonesa
  de aventura/fantasía. **Solo para la demo**: no compromete la ambientación
  del juego final. Los paisajes de CraftPix quedan en reserva.
- **Textos en pantalla:** en **inglés** (título, créditos, intro, "PRESS
  START"). Excepción: la dedicatoria va en español, son las palabras del autor.
  El código y sus comentarios siguen en español.
- **Fuente de texto:** **Ark Pixel Font** (TakWolf, licencia OFL, en
  `res/descargas/fuentes/`; decisión del 2026-09-24). Se usa en el logo del
  título (16 px) y en los textos fijos. Para la consola de texto (celdas de
  8×8) se generó una fuente bitmap de 96 caracteres desde la variante de 10 px:
  las mayúsculas y dígitos miden 7×7 y caben; las minúsculas se muestran como
  mayúsculas (`res/fuente/ark_consola.bmp`, tarea 1b).
- **Vídeo:** NTSC 60 Hz, 256×224, **Mode 1** como base (BG1/BG2 de 16 colores,
  BG3 de 4 colores). **Mode 7** solo en la pantalla de desarrolladores.
  **Mode 3** (BG1 de 256 colores) en las tres escenas fijas de la intro, para
  conservar la fidelidad de los fondos descargados (decisión del 2026-09-24).

## 3. Secuencia de pantallas

Orden final (convención de época: Nintendo → publisher → desarrollador):

```text
 [1] NINTENDO ──► [2] PUBLISHER ──► [3] DEVELOPERS ──► [4] DEDICATION
                                                              │
                                                              ▼
       ┌──────────────────────────────────────────── [5] INTRO (3 escenas)
       │                                                      │
       │  attract: 15 s sin input                             ▼ mosaico
       └───────────────────────────────────────────── [6] TITLE ◄──────┐
                                                              │ Start   │ Start
                                                              ▼         │
                                                     [7] NIVEL 1 ────────┘
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
| 7 Nivel 1 (antes Coming soon) | hasta START | fade in | fade out → título |
| "PRESS START" | parpadeo 0,5 s encendido / 0,5 s apagado | | |

Todos los tiempos se expresan en **frames** en el código (60 frames = 1 s en
NTSC) y se ajustan a ojo en Mesen.

### Pantalla 1 — Nintendo

- **Contenido:** logo "Nintendo" rojo centrado sobre fondo negro y, debajo, el
  texto "LICENSED BY NINTENDO". Muda (como casi todas las de la época).
- **Técnica:** Mode 1. BG1 = imagen del logo (16 colores, 4bpp). Texto con la
  fuente de consola en BG3 (o dentro de la misma imagen si se prefiere una sola
  pieza). `setFadeEffectEx(FADE_IN, 2)` → espera → `setFadeEffectEx(FADE_OUT, 2)`
  (el parámetro es frames por paso de brillo; 16 pasos × 2 = 32 frames).
- **Asset:** `res/nintendo/logo_nintendo.png` (256×224, indexado, ≤16 colores).
  Lo descarga el usuario; se adapta con `scripts/preparar_imagen.py`.

### Pantalla 2 — Publisher (provisional: Capcom)

- **Contenido:** logo de Capcom con jingle, tomado del ejemplo del SDK
  `tools/pvsneslib/snes-examples/thirdparty/logo/snes-logo-capcom/`.
- **Técnica:** Mode 1, `setMode(BG_MODE1, BG3_MODE1_PRIORITY_HIGH)`, fade,
  `spcLoad(MOD_LOGO_PUBLISHER)` + `spcPlay(0)`.
- **Assets:** `res/publisher/logo_publisher.bmp` (96×32 en el ejemplo) y
  `res/musica/jingle_publisher.it`. Reemplazables.

### Pantalla 3 — Developers (logo fijo; Mode 7 descartado)

- **Contenido (decisión del 2026-09-24):** logo propio "DON POLLO" / "STUDIOS"
  en dorado con borde y barra de época, fijo sobre negro; debajo, "ALEJANDRO
  MORA" y "PROGRAMMING - ART - MUSIC" con la consola de texto. Aparece de
  golpe y sale con fade. 4 s.
- **Técnica:** Mode 1, imagen de 16 colores en el fondo 1 (paleta 1), igual
  que Nintendo y publisher. Generada por `scripts/componer_escenas.py` con Ark
  Pixel 16 px.
- **Mode 7, descartado:** se intentó la entrada con zoom y rotación. Con las
  funciones del SDK (`setMode7`/`setMode7Rot`) la imagen no aparecía o salía
  repetida por los bordes, y escribiendo los registros a mano salía ampliada y
  girada 90°. El usuario decidió el logo fijo ("queda mal, solo pon el logo").
  Detalles en §10 por si se retoma.
- **Assets:** `res/devs/logo_devs.png` (256×224, 16 colores).

### Pantalla 4 — Dedication

- **Contenido:** texto centrado sobre negro que aparece letra por letra
  (máquina de escribir), con música suave. Texto **pendiente**: el usuario lo
  entregó el 2026-09-24: **"A la memoria de Tarou"**, en español por ser sus propias palabras.
- **Técnica:** Mode 1, solo BG3 con la fuente de consola. Fade in y fade out
  lentos (60 frames). Rutina común `texto_maquina_escribir()` en `comun.c`.
- **Assets:** ninguno gráfico; `res/musica/dedicatoria.it`.

### Pantalla 5 — Intro (3 escenas)

- **Contenido:** tres escenas, cada una con un fondo distinto y un párrafo
  corto en inglés que aparece con efecto máquina de escribir en una franja
  inferior. Redacta Claude un marcador (presentación de Tarou y Bárbara → el
  hecho que dispara el viaje → el camino por delante); el usuario lo corrige.
  El guion definitivo no está escrito y estos textos no lo comprometen.
- **Sprites (decisión del 2026-09-24, solo para la demo):** **Tarou** usa el
  pack _Wolf Samurai_ (lobo con katana, cuadros de 192×64, se recorta al
  cuerpo y se arma con 2 o 3 sprites de 32×32 o 64×64) y **Bárbara** aparece
  con el pack _Samurai #4_ (mujer samurái, cuadros de 96×96, se recorta a
  64×64). Ambos en 16 colores, animación de reposo de 4 a 6 cuadros. El
  `dogpack` de cachorro queda en reserva para el juego final.
- **Técnica:** **Mode 3**. BG1 = fondo de la escena (256 colores, 8bpp,
  regla `-s 8 -o 256 -u 256 -p -m`). BG2 (16 colores) = texto sobre una franja
  oscura. Al salir hacia el título se vuelve a Mode 1. Fade
  entre escenas. Salida hacia el título con **mosaico**
  (`setMosaicEffect(MOSAIC_IN, MOSAIC_BG1)`).
- **Assets:** `res/intro/escena_1.png` (_Spring Forest_ completo: herrería,
  cerezos, torii = el pueblo), `escena_2.png` (_Autumn Forest_: bosque
  otoñal oscuro, pozo y farolillos = el viento), `escena_3.png` (capas de
  cielo y árboles de _Spring Forest_ sin la herrería, con el torii = el
  camino); 256×224, indexado, 256 colores cada uno, `res/intro/tarou_cachorro.png`
  y `res/intro/barbara.png` (spritesheets, indexado 16 colores),
  `res/musica/intro.it`.

### Pantalla 6 — Title

- **Contenido:** logo del título "Tarou: A Dog's Journey", fondo con
  **parallax** de dos capas, **cielo con degradado HDMA**, Tarou (lobo) y
  Bárbara (samurái) animados en reposo, "PRESS START" parpadeando y la línea "© 2026 [ESTUDIO]"
  al pie. Música en bucle.
- **Técnica:**
  - BG1 = primer plano (scroll rápido, `bgSetScroll` cada frame).
  - BG2 = fondo lejano (scroll lento). El parallax se hace por software con
    dos velocidades; **no** con `setParallaxScrolling`, porque esa función usa
    el canal HDMA 3 y chocaría con el degradado.
  - Degradado: `setModeHdmaGradient()` sobre el color de fondo (canal 3).
  - BG3 = logo del título (4 colores: 3 + transparente) + textos. Mejora
    futura: logo como sprites de 32×32 (16 colores).
  - Tarou = OAM, misma spritesheet que la intro.
  - Attract: contador de frames sin input; a los 900 frames (15 s) →
    fade out → Intro.
  - Start (`padsDown(0) & KEY_START`) → fade out → Nivel 1.
- **Assets:** `res/titulo/logo_titulo.png` (generado por
  `scripts/generar_logo_titulo.py` a partir de una fuente libre; reemplazable),
  `res/titulo/fondo_cerca.png` (cerezos y suelo) y `res/titulo/fondo_lejos.png`
  (cielo y montañas), compuestos desde las 4 capas de _Spring Forest_
  (256×224 o 512×224 para scroll, ≤16 colores cada uno), `res/musica/titulo.it`.
  En el título aparecen los dos sprites: Tarou (lobo) y Bárbara (samurái).

### Pantalla 7 — Nivel 1 (antes "Coming soon")

- **Hasta el 2026-10-16:** "LEVEL 1 — COMING SOON" centrado sobre negro y
  "PRESS START TO RETURN", en Mode 1 solo con texto: el **enchufe** para el
  gameplay (`pantalla_proximamente.c`).
- **Desde la Revisión 1:** `pantalla_nivel1.c` carga el nivel 1 (Tarou adulto,
  parallax, HUD de depuración). Contenido, técnica y verificación en
  `docs/diseno-demo-jugable.md`; esta pantalla ya no forma parte de la
  secuencia de arranque propiamente dicha.

## 4. Arquitectura del código

```text
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
  pantalla_nivel1.c       el nivel 1 (desde la Revisión 1; ver docs/diseno-demo-jugable.md)
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

```text
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
- `scripts/generar_logo_titulo.py`: renderiza "TAROU" y "A Dog's Journey" con
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
3. En el título: Start → fade → nivel 1; START en el nivel → fade → título
   con la música reiniciada.
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
| Q7 | Start → fade → "LEVEL 1 — COMING SOON" → Start → título. (Sustituido por el nivel 1 en la Revisión 1.) |
| Q8 | Attract a los 15 s, vuelve a la intro. |
| Q9/Q17 | Proyecto personal sin distribución; se usa material original con copyright; logos aislados como assets reemplazables. Premisa del plan actualizada. |
| Q10/Q23 | Logo de estudio provisional estilo Konami + "ALEJANDRO MORA" y roles. |
| Q11/Q26 | Intro: 3 escenas, texto máquina de escribir (marcador redactado por Claude), Tarou sprite en la 3.ª. |
| Q12 | Estética _Secret of Mana_. |
| Q13/Q20 | Assets descargados por el usuario a `res/descargas/`; script Python de indexado; el arte debe exprimir la SNES. |
| Q14 | Textos en pantalla en inglés; comentarios en español. |
| Q15/Q19 | Audio: aún no descargado; carpetas listas; mientras, `.it` de los ejemplos del SDK. |
| Q16 | Nueva Etapa 1.5 en el plan. |
| Q22 | Nintendo: logo + "LICENSED BY NINTENDO", muda, fade. |
| Q24 | Publisher provisional: Capcom del SDK. |
| Q25/Q33 | Dedicatoria: texto sobre negro, 7 s, música suave; texto pendiente, marcador "For ...". |
| Q27 | Logo del título generado por script desde fuente libre. |
| Q28 | Título: parallax + HDMA + Tarou animado + PRESS START parpadeante + © + música. |
| Q29 | Efectos: fade, mosaico, degradado HDMA, parallax, Mode 7 (devs). Transparencia y ventanas para el gameplay. |
| Q30 | Tiempos según tabla de §3. |
| Q31 | Un `.c` por pantalla + `secuencia.c` con enum/switch + `main.c` mínimo. |
| Q32 | Este documento + resumen en el plan. |
| Q34 | `res/`, rama `demo-secuencia-arranque`, Mode 1 base, fuente SDK provisional, solo Tarou cachorro, verificación en Mesen, CLAUDE.md actualizado. |
| Q35 | Implementación arranca en la misma sesión: estructura, carpetas, máquina de estados y pantalla Nintendo con marcadores. |

## 8. Pendientes que dependen del usuario

- [x] Texto de la dedicatoria: **"A la memoria de Tarou"** (en español, palabras del autor; 2026-09-24).
- [x] Nombre del estudio: **Don Pollo Studios** (2026-09-24).
- [x] Descargas en `res/descargas/graficos/` (2026-09-24): logo Nintendo
      (Wikimedia Commons), fondos _Spring Forest_ y _Autumn Forest_ (intro y
      título), sprites _Wolf Samurai_ (Tarou) y _Samurai #4_ (Bárbara). En
      reserva: CraftPix Nature Landscapes, Village landscape, `dogpack_assets`.
      Logo de estudio: Konami del SDK.
- [~] Descargas en `res/descargas/audio/`: dos `.it` que pasan `smconv` sin
      avisos (`aguadulc.it`, `ad_jon.it`, ver `candidatos_it/NOTA.md`) más los
      del SDK; falta elegirlos oyéndolos en Mesen. Los packs MP3/OGG/WAV
      descargados no sirven como música de SNES.
- [ ] Corregir los tres párrafos marcador de la intro.
- [ ] Música japonesa: solo "Oriental 2" (`mw_orie2.it`) pasa `smconv`. Los
      packs MP3/OGG no sirven. Alternativas: seguir buscando `.it` pequeños,
      o adaptar uno grande en OpenMPT (bajar a 8 canales, muestras sin
      comprimir).

## 9. Plan de implementación (una tarea = un commit)

| # | Tarea | Qué se ve en Mesen al terminar | Estado |
| --- | --- | --- | --- |
| 1 | **Esqueleto**: carpetas `res/` y `scripts/`, `.gitignore`, `scripts/preparar_imagen.py`, `docs/glosario-snes.md`, máquina de estados `secuencia.c`, las 7 pantallas con **texto marcador** y los tiempos reales, fades, parpadeo, attract y Start funcionando. | La secuencia completa corre de punta a punta solo con texto. | **hecho** (2026-09-23). Vista en Mesen el 2026-09-24: fundidos de 8 s por un error de unidades (frames por paso, no totales), corregido. |
| 1b | **Fuente Ark Pixel en la consola**: `scripts/generar_fuente_consola.py` (10 px → BMP 768×8, mayúsculas), regla `gfx4snes` en el Makefile, `data.asm` con la sección de la fuente, `consoleInitText` en `main.c`. | Todo el texto de la demo en Ark Pixel. | **hecho** (2026-09-24) |
| 1c | **Sprites de Tarou y Bárbara**: `scripts/preparar_sprites.py` (tiras de 4 cuadros de 64×64, 16 colores), `src/personajes.c` (carga única en VRAM 0x0000, animación de reposo, lobo volteado), visibles en la escena 3 y el título. Límite duro: 512 tiles de sprites = 8 bloques de 64×64. | Los dos personajes animados en la intro y el título. | **hecho** (2026-09-24) |
| 2 | **Nintendo real**: `res/nintendo/logo_nintendo.png` → `gfx4snes` → `data.asm` → `bgInitTileSet`/`bgInitMapSet` en BG1; texto "LICENSED BY NINTENDO" en BG3. | Logo rojo con fade. | **hecho** (2026-09-24) |
| 3 | **Publisher + audio**: logo Capcom del SDK, primer `.it` → `soundbank` (`AUDIOFILES`, `SOUNDBANK`, `spcBoot`, `spcLoad`, `spcPlay`, `spcProcess` en el bucle). | Logo Capcom (sin jingle aún: falta el audio). | **hecho** parcial (2026-09-24), audio pendiente |
| 4 | **Devs en Mode 7**: imagen 256 colores (`-M 7`), `bgInitMapTileSet7`, zoom + rotación por frame (`m7sx/m7sy`, `setMode7Rot`), vuelta limpia a Mode 1. | Logo propio "DON POLLO STUDIOS" fijo. **Mode 7 descartado** el 2026-09-24: la PPU lo rotaba/escalaba mal con la librería y se decidió el logo fijo. | **hecho** (2026-09-24) |
| 5 | **Dedicatoria**: música suave, texto definitivo, fades lentos. | Texto máquina de escribir con música. | pendiente (texto del usuario) |
| 6 | **Intro**: 3 fondos en BG1, franja de texto en BG3 con prioridad alta, sprite de Tarou (`oamInitGfxSet`, `oamSet`) animado en la escena 3, salida con mosaico. | Tres escenas japonesas (Spring/Autumn Forest), escenas 1-2 en Mode 3, escena 3 en Mode 1 con los dos sprites. | **hecho** (2026-09-24) |
| 7 | **Título**: `scripts/generar_logo_titulo.py` → logo en BG3, parallax por software en BG1/BG2, `setModeHdmaGradient`, sprite de Tarou, música en bucle. | Título con parallax (arbustos), montañas, cielo HDMA, logo Ark Pixel, sprites, PRESS START. | **hecho** (2026-09-24) |
| 7b | **Pulido visual** (pedido del usuario el 2026-09-24: "tenemos que mejorar todo después"): revisar composición de cada pantalla, logo del estudio, colores, tiempos, y las mejoras futuras anotadas en cada sección. | Demo con acabado. | pendiente |
| 8 | **Cierre**: pase completo de verificación (§6), `snesromusage` por bancos, ajuste de tiempos a ojo, merge de `demo-secuencia-arranque` a `main`. | Demo terminada. | pendiente |

Cada tarea termina con `make clean` + `make` sin errores (el SDK no detecta
cambios en headers), la ROM vista en Mesen y un commit. Lecciones del
2026-09-24 verificadas en el ensamblador del SDK: `setFadeEffectEx` cuenta
frames por paso de brillo (16 pasos); la documentación de `setMosaicEffect`
está invertida (`MOSAIC_OUT` agranda y deja el mosaico activo, se apaga con
`REG_MOSAIC = 0`); el color 0 de la paleta 0 es el fondo de toda la pantalla. Las tareas 2, 4, 6 y 7 pueden reordenarse según qué asset llegue
primero a `res/descargas/`.

## 10. Hallazgos técnicos verificados (2026-09-24)

Todo esto se comprobó leyendo el ensamblador del SDK (`videos.asm`,
`backgrounds.asm` en GitHub; el vendorizado solo trae binarios) o midiendo en
Mesen. Sirve para cualquier pantalla futura.

**Compilación**

- `snes_rules` compila un `.c` solo si cambió ESE archivo: un cambio en un `.h`
  no recompila a quienes lo incluyen. Tras tocar headers: `make clean`.
- `data.asm` incrusta los assets con `.incbin`; `make` no lo reensambla si el
  `.asm` no cambió. El Makefile declara `data.obj: $(ASSETS)`.
- `gfx4snes -y` (mapa en páginas de 32×32) se cae si el alto no es múltiplo de
  32: la capa cercana del título es de 512×256, no 224.
- `gfx4snes -M 7` con una imagen de 256×224 produce un mapa de 32×28, no el de
  128×128 que espera la VRAM de Mode 7: hay que darle 1024×1024.
- Un `.pic` de 256 colores de más de 512 tiles supera los 32 KB de un banco:
  las escenas Mode 3 miden 256×120 (480 tiles) + banda transparente.

**PPU y librería**

- `setFadeEffectEx(modo, n)`: `n` son frames POR PASO de brillo y hay 16
  pasos. 2 → 32 frames (~0,5 s). Con 30 cada fundido duraba 8 s.
- `setMosaicEffect`: la doc está invertida. `MOSAIC_OUT` agranda 0→15 y deja
  el mosaico activo; `MOSAIC_IN` lo encoge 15→0. Se apaga con `REG_MOSAIC = 0`.
- El color 0 de la paleta 0 (y el índice 0 de una imagen de 256 colores) es el
  color de fondo de TODA la pantalla: debe ser negro, no el magenta "de
  transparencia" que traen los BMP del SDK.
- La OAM direcciona 512 tiles de sprites (dos tablas de 256). Con sprites de
  64×64 (64 tiles cada uno) caben 8 cuadros en total. Por encima, los números
  "dan la vuelta" y se ve otro sprite.
- Esconder un sprite en y = 240 no basta si mide 64 de alto: la coordenada da
  la vuelta cada 256 px y asoman 48 filas por arriba. Se esconde con tamaño
  pequeño, y = 224 y x = 256 (bit alto = fuera por la derecha).
- `gfx4snes` empaqueta los bloques de sprites en filas de 128 px (16 tiles):
  bloque n de 64×64 → tile `(n/2)*128 + (n%2)*8`.
- En Mode 3 el texto va en BG2 (16 colores) con paleta 7 (colores 112..127) y
  las imágenes reservan esas 16 entradas. El texto queda DETRÁS de BG1: la
  zona de texto de la imagen debe ser índice 0 (transparente).
- El cielo del título es `setModeHdmaColor(HDMA_CHANNEL6, tabla)` con la tabla
  generada por `gfx4snes -n` de `res/titulo/cielo.bmp` (un color por línea);
  el símbolo se llama `hdmaGrad<nombre>List`. `setParallaxScrolling` y
  `setModeHdmaGradient` usan el canal 3 y son otra cosa (brillo, no color).
- `setMode7` de PVSnesLib fija M7VOFS = 384 y el centro en (128,128): la
  ventana visible son las filas 384..607 del mapa y el pivote queda fuera de
  pantalla. `setMode7Rot` calcula la matriz como `(m7sx * cos) >> 8` con una
  tabla de senos de amplitud 127: tamaño real = `m7sx ≈ 0x200`. Escribiendo
  los registros a mano (matriz 8.8, centro 128,112, HOFS/VOFS 0) la imagen se
  vio ampliada y rotada 90°: pendiente de entender si se retoma Mode 7.
- El `companyLogo.bmp` de Konami del SDK no es una imagen: es un juego de tiles
  que su ejemplo reordena con un mapa propio. No sirve como fondo directo.

**Verificación**

- Capturar Mesen desde PowerShell solo sirve si su ventana está delante; si el
  usuario está usando el equipo, las capturas salen de otra ventana.
- `scripts/capturas_mesen.lua` toma capturas exactas cada 4 s desde dentro de
  Mesen, pero requiere activar "Allow I/O and OS access" en la ventana de
  scripts de Mesen (`AllowIoOsAccess` en su `settings.json`, por defecto
  `false`). Mesen sí carga un `.lua` pasado en la línea de comandos.
