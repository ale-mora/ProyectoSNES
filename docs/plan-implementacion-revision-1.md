# Plan de implementación — Revisión 1: Tarou adulto en el nivel 1

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Objetivo:** dejar el repositorio con la estructura del juego completo (código del MVP más stubs documentados), la Revisión 1 jugable y verificada en Mesen, la documentación de cada archivo y su conexión con la cadena de herramientas y el hardware, y el plan de la carpeta `aprendizaje/`.

**Arquitectura:** la máquina de estados de `secuencia.c` gana la pantalla `PANTALLA_NIVEL1` (sustituye a «coming soon») y una tercera función por pantalla, `_vblank()`. La lógica del protagonista (`tarou.c`, `camara.c`, `nivel.c`, `animaciones.c`) no conoce el hardware y se prueba en PC con `gcc`; la capa de hardware (`mando.c`, `tarou_sprite.c`, `hud_debug.c`, `pantalla_nivel1.c`) traduce a mando, OAM dinámica y texto. Un pipeline en Python convierte el arte de Pixellab (o marcadores) en tiras indexadas que `gfx4snes -T` vuelve metasprites.

**Tecnología:** PVSnesLib (`tcc816`, `wla-65816`, `gfx4snes`), Mesen, Python 3 con Pillow, `gcc` (WinLibs/MinGW, ya en PATH) y `make` de MSYS2 para las pruebas en PC.

**Spec:** `docs/diseno-demo-jugable.md` (v2). El plan cita sus secciones como «§n». Vocabulario en `docs/glosario-snes.md`.

**Nota de alcance:** el plan tiene ocho fases. Cada fase deja software que compila y funciona en Mesen; pueden ejecutarse en sesiones distintas. Las fases 3 y 4 son el corazón; 6 y 7 son documentación y no tocan código.

## Restricciones globales

- Cada línea de código lleva comentario en español; texto en pantalla del juego en inglés (CLAUDE.md). El HUD de depuración usa los nombres del código (§1).
- Todo archivo fuente nuevo o tocado empieza con la cabecera estándar de §12; los stubs añaden `PENDIENTE: Etapa N — …`.
- Estilo de formato: el de `.clang-format` (Tarea 0.1); no se realinean comentarios a mano.
- LoROM, SlowROM, **16 bancos** (`ROMSIZE := 09`, `ROMBANKS := 16`) a partir de la Tarea 0.2.
- Lógica pura (`tarou.c`, `camara.c`, `nivel.c`, `animaciones.c`): solo `u8/u16/s8/s16`, sin `int`, `long`, `float`, `bool`, punteros a función ni `snes.h`; incluye `sistema/tipos.h`. Posiciones nunca negativas (§5).
- Nombre del personaje: **Tarou** en todo; `Tarouh` no debe quedar en ningún archivo versionado salvo en el historial de `docs/diseno-demo-jugable.md` §0.
- No editar nada bajo `tools/`. `hdr.asm` y `linkfile` son generados.
- Tras tocar cualquier `.h`: `make clean` antes de `make` (snes_rules no rastrea cabeceras).
- Compilar la ROM desde PowerShell: `$env:PVSNESLIB_HOME="/e/ProyectoSNES/tools/pvsneslib"; $env:PATH="E:\ProyectoSNES\tools\pvsneslib\devkitsnes\bin;C:\msys64\usr\bin;"+$env:PATH; make` (en adelante, «**compilar la ROM**»). Pruebas en PC: `make -C tests && .\tests\pruebas.exe` («**correr las pruebas**»).
- Commits: uno por entregable, mensaje en español, sin push. Cada mensaje termina con `Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>`.

## Puntos de atención (Review Focus)

Condiciones que la spec implica pero que es fácil romper. Cada una tiene su prueba en la tarea indicada.

1. **Borde izquierdo:** caminar a la izquierda desde `x_fp = 0` deja `x_fp = 0` y `vx_fp = 0` sin que la conversión `u16` desborde (§5 «posiciones nunca negativas»). Prueba en Tarea 3.4.
2. **Soltar B en el primer frame del salto:** `vy_fp` pasa a `VEL_SALTO_CORTE` (−32), no a 0, y el salto dura más de un frame. Prueba en Tarea 3.5.
3. **R y A el mismo frame con la espada fuera:** gana R (`ENVAINANDO`), no `ATACANDO` (§4 prioridad). Prueba en Tarea 3.6.
4. **Actividad durante `ECHANDOSE`:** pasa a `DESPERTANDO` sin visitar `DORMIDO`, y `DESPERTANDO` ignora el mando hasta terminar su animación. Prueba en Tarea 3.7.
5. **Tira con 16 cuadros:** el pipeline termina con error y no escribe ninguna salida (§7 presupuesto). Prueba en Tarea 2.2.

---

## Fase 0 — Pendientes y preparación

### Tarea 0.1: Cerrar lo pendiente en la rama

**Archivos:**
- Modificar: `.gitignore` (añadir `*.exe`)
- Crear: `.clang-format`
- Ya modificados sin commit: `CLAUDE.md`, `src/pantalla_dedicatoria.c`, `src/pantalla_intro.c`, `docs/glosario-snes.md`, `docs/diseno-demo-jugable.md`
- Crear (nuevo en git): `docs/plan-implementacion-revision-1.md`, `aprendizaje/`, `examples/likemario/`

- [ ] **Paso 1: Escribir `.clang-format`** con `BasedOnStyle: LLVM`, `IndentWidth: 4`, `UseTab: Never`, `BreakBeforeBraces: Allman`, `IndentCaseLabels: false`, `ColumnLimit: 0`, `AlignTrailingComments: true`, `SpacesBeforeTrailingComments: 1`.
- [ ] **Paso 2: Verificar que el estilo coincide con el reformateo aceptado**

Ejecutar (PowerShell): `& "$env:USERPROFILE\.vscode\extensions\ms-vscode.cpptools-1.34.4-win32-x64\LLVM\bin\clang-format.exe" -i src/pantalla_dedicatoria.c; git diff --stat src/pantalla_dedicatoria.c`
Esperado: `git diff --stat` no muestra cambios nuevos respecto al archivo ya reformateado (si los muestra, ajustar `.clang-format` hasta que no).
- [ ] **Paso 3: Añadir `*.exe` a `.gitignore`** bajo «Artefactos de compilación».
- [ ] **Paso 4: Compilar la ROM** (sin `make clean`). Esperado: `ProyectoSNES.sfc` generado, sin errores.
- [ ] **Paso 5: Commit**

```bash
git add .gitignore .clang-format CLAUDE.md src/pantalla_dedicatoria.c src/pantalla_intro.c docs/ aprendizaje/ examples/likemario/
git status --short   # comprobar que no entra ningún .exe, .obj, .sfc ni .sym
git commit -m "Cerrar pendientes: dedicatoria a Tarou, formato de código, material de aprendizaje y diseño v2 de la demo jugable"
```

### Tarea 0.2: ROM de 16 bancos

**Archivos:**
- Modificar: `Makefile` (antes del `include ${PVSNESLIB_HOME}/devkitsnes/snes_rules`)
- Modificar: `CLAUDE.md` («8 ROM banks» → 16; «256 KB» → 512 KB), `docs/glosario-snes.md` (entrada Banco / LoROM: «Nuestro proyecto tiene 8 (256 KB)» → 16 (512 KB))

- [ ] **Paso 1: Añadir al Makefile** `export ROMSIZE := 09` y `export ROMBANKS := 16` con un comentario de por qué (§2 decisión 4: ~110 KB de cuadros).
- [ ] **Paso 2: `make clean` y compilar la ROM.**
- [ ] **Paso 3: Verificar** `grep "OBTAIN_ROMBANKS" ProyectoSNES.log` → `(16)`; `ProyectoSNES.sfc` mide 524288 bytes; `hdr.asm` contiene `.ROMBANKS 16` y `ROMSIZE $09`.
- [ ] **Paso 4: Lanzar Mesen** con la ROM y comprobar que la secuencia de arranque llega al título (la cabecera de la ROM es válida).
- [ ] **Paso 5: Actualizar CLAUDE.md y el glosario** con el nuevo tamaño.
- [ ] **Paso 6: Commit** — `git commit -m "ROM de 512 KB (16 bancos) para los cuadros del protagonista"`

---

## Fase 1 — Reorganizar y renombrar (sin cambiar comportamiento)

### Tarea 1.1: Subcarpetas de `src/`

**Archivos:**
- Mover: `src/main.c`, `src/secuencia.c/.h`, `src/comun.c/.h`, `src/pantallas.h` → `src/sistema/`
- Mover: `src/pantalla_*.c`, `src/personajes.c/.h` → `src/pantallas/`
- Modificar: `Makefile` (`CFLAGS += -Isrc` después del `include` de `snes_rules`), todos los `#include "…"` de los archivos movidos, `.vscode/c_cpp_properties.json` (`includePath` con `${workspaceFolder}/src`)

**Interfaces:**
- Produce: la convención de inclusión `#include "sistema/comun.h"`, `#include "pantallas/personajes.h"` que usan todas las tareas siguientes.

- [ ] **Paso 1: `git mv`** de cada archivo a su carpeta (los `.obj` no se mueven; `make clean` los borra).
- [ ] **Paso 2: Añadir `CFLAGS += -Isrc`** al Makefile, tras el `include` de `snes_rules`, con un comentario que explique que las recetas expanden `CFLAGS` al ejecutarse.
- [ ] **Paso 3: Reescribir los `#include`** con ruta desde `src/` (`"sistema/secuencia.h"`, `"sistema/comun.h"`, `"sistema/pantallas.h"`, `"pantallas/personajes.h"`).
- [ ] **Paso 4: `make clean` y compilar la ROM.** Esperado: sin errores; `ProyectoSNES.log` lista `src/sistema/main.obj`, etc.
- [ ] **Paso 5: Mesen:** la secuencia completa se ve igual que antes (Nintendo → … → título → coming soon → título).
- [ ] **Paso 6: Commit** — `git commit -m "Organizar src/ en sistema/ y pantallas/"`

### Tarea 1.2: Renombrar Tarouh → Tarou y el lobo → lobo_samurai

**Archivos:**
- Renombrar: `res/personajes/tarouh_lobo.png` → `res/personajes/lobo_samurai.png`
- Modificar: `Makefile` (regla y `ASSETS`), `data.asm` (sección `.rodata_lobo`, etiquetas `lobo_samurai_til/_til_end/_pal`), `src/pantallas/personajes.c/.h` (externs, `OAM_TAROU`, `PALETA_TAROU`, comentarios: «Tarou» es el personaje, «lobo samurái» el sprite provisional que será enemigo), `src/pantallas/pantalla_titulo.c` (comentario), `src/sistema/main.c` (cabecera), `Makefile` (`ROMTITLE := TAROU DEMO`), `docs/plan-desarrollo.md`, `docs/diseno-secuencia-arranque.md`, `CLAUDE.md`, `aprendizaje/**/*.c`, `examples/likemario/LikeMario.md`

- [ ] **Paso 1: Listar las apariciones** — `git grep -n -i "tarouh"` y guardar la cuenta.
- [ ] **Paso 2: `git mv` del PNG** y reemplazo en Makefile y `data.asm`.
- [ ] **Paso 3: Reemplazar `Tarouh` → `Tarou` en todos los archivos versionados** (texto y código; el título del juego pasa a _Tarou: A Dog's Journey_).
- [ ] **Paso 4: Verificar** — `git grep -n -i "tarouh"` solo devuelve la fila de historial de `docs/diseno-demo-jugable.md` §0 y la tabla de decisiones (decisión 17).
- [ ] **Paso 5: `make clean`, compilar la ROM, Mesen:** el título dice lo mismo salvo el nombre; sprites iguales.
- [ ] **Paso 6: Commit** — `git commit -m "Renombrar el protagonista a Tarou; el lobo samurai pasa a ser un sprite de enemigo"`

### Tarea 1.3: Cabecera estándar en los archivos existentes y gancho `_vblank`

**Archivos:**
- Modificar: los 15 archivos de `src/sistema/` y `src/pantallas/` (cabecera §12), `data.asm` (cabecera con `;`), `src/sistema/main.c` (bucle), `src/sistema/secuencia.c/.h`, `src/sistema/pantallas.h`

**Interfaces:**
- Produce: `void secuencia_vblank(void);` en `secuencia.h`, llamada por `main.c` justo después de `WaitForVBlank()`; su `switch` llama a `pantalla_X_vblank()` solo para las pantallas que la declaren (de momento ninguna: `default: break;`).

- [ ] **Paso 1: Reescribir la cabecera** de cada archivo con las cinco secciones de §12, conservando el texto explicativo existente dentro de PROPÓSITO. La tabla VARIABLES lista cada `static` y global del archivo con tipo, rango y uso.
- [ ] **Paso 2: Añadir `secuencia_vblank()`** y la llamada en `main.c`: `secuencia_actualizar(); WaitForVBlank(); secuencia_vblank();` con el comentario de por qué existe (§7: trabajo que debe hacerse justo después del VBlank).
- [ ] **Paso 3: `make clean`, compilar la ROM, Mesen:** sin cambios visibles.
- [ ] **Paso 4: Commit** — `git commit -m "Cabecera estandar en todos los fuentes y gancho secuencia_vblank"`

---

## Fase 2 — Pipeline de arte del protagonista

### Tarea 2.1: Carpetas de descarga de Pixellab con instrucciones

**Archivos:**
- Crear: `res/descargas/graficos/pixellab/tarou/README.md`, `res/descargas/graficos/pixellab/barbara/README.md`
- Modificar: `.gitignore` (excepciones: `!res/descargas/graficos/pixellab/`, `!res/descargas/graficos/pixellab/*/`, `!res/descargas/graficos/pixellab/*/README.md`; todo lo demás dentro sigue ignorado)

- [ ] **Paso 1: Escribir el README** (en español) con: las 13 animaciones y su nombre de archivo exacto (`reposo`, `caminar`, `correr`, `saltar`, `caer`, `agachado`, `agachado_caminar`, `desenvainar`, `envainar`, `atacar`, `echarse`, `dormido`, `despertar`); los tres formatos aceptados (tira PNG `nombre.png`, carpeta `nombre/01.png…`, `nombre.gif`); las reglas del hardware (64×64, ≤ 15 colores + transparente, una paleta, mirando a la derecha, pies en la misma fila, «caminar en el sitio», máximo 15 cuadros por animación); cómo exportar desde Pixellab (Characters → exportar como «sprite sheet» o «individual frames», según su [documentación](https://www.pixellab.ai/docs/ways-to-use-pixellab)); y el comando `python scripts/preparar_personaje.py tarou`.
- [ ] **Paso 2: Verificar el gitignore** — `git status --short` muestra los dos README como nuevos; un PNG de prueba dentro de `pixellab/tarou/` no aparece (borrarlo después).
- [ ] **Paso 3: Commit** — `git commit -m "Carpetas de descarga de Pixellab con instrucciones para Tarou y Barbara"`

### Tarea 2.2: `scripts/preparar_personaje.py`

**Archivos:**
- Crear: `scripts/preparar_personaje.py`
- Crear (salida versionada): `res/personajes/tarou/tarou_<anim>.png` ×13, `res/personajes/tarou/tarou_anim.h`
- Crear: `scripts/pruebas/prueba_preparar_personaje.py` (pruebas con `unittest`, sin dependencias extra)

**Interfaces:**
- Produce: `tarou_anim.h` con `#define TAROU_<ANIM>_CUADROS n` para las 13 animaciones (`ANIM` en mayúsculas: `REPOSO`, `CAMINAR`, …, `AGACHADO_CAMINAR`, …) y `#define TAROU_ANIMACIONES 13`.
- Produce: tiras `tarou_<anim>.png` indexadas (modo `P`), 64 px de alto, ancho `64 * n`, índice 0 transparente, misma paleta en las 13.

- [ ] **Paso 1: Escribir las pruebas** en `scripts/pruebas/prueba_preparar_personaje.py` (se ejecutan con `python -m unittest scripts/pruebas/prueba_preparar_personaje.py` desde la raíz):
  - `test_marcador_cuando_falta_animacion`: con una carpeta de entrada vacía, la salida tiene 13 PNG de 64 px de alto y `tarou_anim.h` define `TAROU_REPOSO_CUADROS 4` (los marcadores usan los cuadros por defecto: reposo 4, caminar 8, correr 8, saltar 2, caer 2, agachado 2, agachado_caminar 6, desenvainar 4, envainar 4, atacar 5, echarse 4, dormido 4, despertar 4).
  - `test_tira_png`: una tira RGBA de 3 cuadros de 64×64 como `reposo.png` produce `tarou_reposo.png` de 192×64 y `TAROU_REPOSO_CUADROS 3`.
  - `test_carpeta_de_cuadros`: `caminar/01.png`, `02.png` → `TAROU_CAMINAR_CUADROS 2`.
  - `test_gif`: un GIF de 2 cuadros como `correr.gif` → `TAROU_CORRER_CUADROS 2`.
  - `test_paleta_compartida`: las 13 salidas tienen el mismo `getpalette()` y ≤ 16 entradas usadas; el píxel (0,0) de un cuadro vacío es índice 0.
  - `test_alineacion_comun`: dos animaciones con el personaje a distinta altura salen con los pies en la fila 63 en ambas.
  - `test_error_mas_de_15_cuadros`: una tira de 16 cuadros hace que el script termine con `SystemExit` distinto de 0 y no exista ninguna salida en la carpeta destino (Punto de atención 5).
- [ ] **Paso 2: Ejecutarlas** — Esperado: fallan por `ModuleNotFoundError` / archivos ausentes.
- [ ] **Paso 3: Implementar** `preparar_personaje.py` con `def preparar(nombre: str, entrada: Path, salida: Path) -> dict[str, int]` (nombre de animación → cuadros) y `main()` con `argparse` (`nombre`, `--entrada`, `--salida` con los valores por defecto de §10). Reutilizar de `preparar_sprites.py` la cuantización a 15 colores con magenta como índice 0 y la caja envolvente; la caja se calcula sobre **todos** los cuadros de todas las animaciones. Marcador: fondo transparente, rectángulo de borde, texto `nombre` y número del cuadro con la fuente por defecto de Pillow. Validar `cuadros <= 15` antes de escribir nada.
- [ ] **Paso 4: Ejecutar las pruebas** — Esperado: 7 OK.
- [ ] **Paso 5: Generar las salidas reales** — `python scripts/preparar_personaje.py tarou` (la carpeta de Pixellab está vacía: salen marcadores). Comprobar a ojo `res/personajes/tarou/tarou_atacar.png` (abrirlo con la herramienta de lectura de imágenes).
- [ ] **Paso 6: Commit** — `git commit -m "Pipeline del protagonista: tiras indexadas con paleta compartida y tarou_anim.h, con marcadores si falta arte"`

### Tarea 2.3: Conversión con gfx4snes y datos en la ROM

**Archivos:**
- Modificar: `Makefile` (regla patrón `res/personajes/tarou/%.pic: res/personajes/tarou/%.png` → `$(GFXCONV) -s 32 -o 16 -u 16 -p -T -X 64 -Y 64 -P 3 -i $<`; los 13 `.pic` en `ASSETS`; `CFLAGS += -Ires/personajes/tarou`), `data.asm` (13 secciones `.rodata_tarou_<anim>` con `tarou_<anim>_til`/`_til_end` y, solo en reposo, `tarou_pal`)
- Modificar: `.gitignore` (nada: `*.pic`, `*.pal`, `*.inc` ya están ignorados; `tarou_anim.h` se versiona)

**Interfaces:**
- Produce: etiquetas `tarou_<anim>_til`, `tarou_<anim>_til_end` (13) y `tarou_pal` para `extern char` en C; archivos `res/personajes/tarou/tarou_<anim>_meta.inc` con `const t_metasprite *tarou_<anim>_metasprites[n]` (verificar el nombre exacto en el Paso 3).

- [ ] **Paso 1: Escribir las reglas y las secciones** con comentarios que expliquen `-T -X 64 -Y 64` (tabla metasprite de 64×64 en piezas de 32) y `-P 3` (prioridad).
- [ ] **Paso 2: Compilar la ROM.** Esperado: 13 `.pic`, 13 `.pal`, 13 `_meta.inc` generados; el `.log` no supera 32 KB en ninguna sección `.rodata_tarou_*`.
- [ ] **Paso 3: Leer un `_meta.inc`** (`tarou_reposo_meta.inc`) y anotar en el comentario de cabecera de `data.asm` el nombre real de las tablas y la numeración de `dtile` de los 4 cuadros (esperado: cuadro 0 = bloques 0, 1, 8, 9 para una tira de 4 cuadros).
- [ ] **Paso 4: Verificar la paleta compartida** — `cmp res/personajes/tarou/tarou_reposo.pal res/personajes/tarou/tarou_atacar.pal` sin salida (idénticos).
- [ ] **Paso 5: Commit** — `git commit -m "Convertir las animaciones de Tarou a metasprites de 32x32 e incrustarlas en la ROM"`

### Tarea 2.4: Fondo lejano del nivel 1

**Archivos:**
- Crear: `scripts/preparar_fondo_nivel.py`, `res/nivel1/fondo_lejos.png` (salida versionada)
- Modificar: `Makefile` (regla `res/nivel1/fondo_lejos.pic` con `-y -s 8 -o 16 -u 16 -e 2 -p -m`; en `ASSETS`), `data.asm` (sección `.rodata_nivel1_lejos`: `nivel1_lejos_til/_til_end/_pal/_map/_map_end`)

- [ ] **Paso 1: Medir el logo** en `res/titulo/fondo_lejos.png` (abrir la imagen; anotar la caja x0,y0,x1,y1 del logo horneado).
- [ ] **Paso 2: Escribir el script** `preparar_fondo_nivel.py ENTRADA SALIDA --borrar x0,y0,x1,y1`: pone la caja al índice 0 (transparente, deja ver el cielo HDMA) y concatena la imagen con su espejo horizontal hasta 512 px, conservando la paleta indexada.
- [ ] **Paso 3: Ejecutar y comprobar a ojo** la salida: 512×H, sin logo, sin costura visible en el centro.
- [ ] **Paso 4: Compilar la ROM.** Esperado: `res/nivel1/fondo_lejos.pic` generado, ≤ 512 tiles (ver el `.log` o el tamaño del `.pic` ≤ 16384 bytes).
- [ ] **Paso 5: Commit** — `git commit -m "Fondo lejano del nivel 1 sin logo y de 512 px"`

---

## Fase 3 — Lógica pura con pruebas en PC

### Tarea 3.1: Tipos portables, punto fijo y arnés de pruebas

**Archivos:**
- Crear: `src/sistema/tipos.h`, `src/sistema/puntofijo.h`
- Crear: `tests/minitest.h`, `tests/pruebas.c`, `tests/prueba_puntofijo.c`, `tests/Makefile`

**Interfaces:**
- Produce: `tipos.h`: con `-DPRUEBAS_PC` define `u8,u16,s8,s16` desde `stdint.h`; sin él incluye `<snes.h>`.
- Produce: `puntofijo.h`: `#define PF_BITS 4`, `#define PF(n) ((s16)((n) << PF_BITS))`, `#define PF_ENTERO(x) ((u16)((x) >> PF_BITS))` (solo para `x >= 0`).
- Produce: `minitest.h`: `extern u16 pruebas_total, pruebas_fallidas;` y `COMPROBAR(cond)` que cuenta e imprime `FALLO archivo:línea: cond`; `pruebas.c` llama a `pruebas_puntofijo()`, `pruebas_camara()`, `pruebas_tarou()`, `pruebas_animacion()` (las que existan), imprime `N pruebas, M fallos` y devuelve `fallidas != 0`.
- Produce: `tests/Makefile` con `CC = gcc`, `CFLAGS = -std=c99 -Wall -Wextra -DPRUEBAS_PC -I. -I../src`, objetivo `pruebas.exe` a partir de `pruebas.c prueba_*.c` y los `.c` puros de `../src/juego/` que existan.

- [ ] **Paso 1: Escribir `tests/prueba_puntofijo.c`** con `void pruebas_puntofijo(void)`: `COMPROBAR(PF(1) == 16)`, `COMPROBAR(PF(448) == 7168)`, `COMPROBAR(PF_ENTERO(PF(100) + 15) == 100)`, `COMPROBAR(PF_ENTERO(PF(100) + 16) == 101)`, `COMPROBAR((s16)(PF(0) - 2) < 0)`.
- [ ] **Paso 2: Correr las pruebas** — Esperado: falla la compilación (no existen los headers).
- [ ] **Paso 3: Implementar** `tipos.h`, `puntofijo.h`, `minitest.h`, `pruebas.c`, `tests/Makefile`.
- [ ] **Paso 4: Correr las pruebas** — Esperado: `5 pruebas, 0 fallos`.
- [ ] **Paso 5: Compilar la ROM** (`make clean` antes: hay cabeceras nuevas). Esperado: sin errores (los headers no se usan todavía).
- [ ] **Paso 6: Commit** — `git commit -m "Tipos portables, punto fijo 12.4 y arnes de pruebas en PC"`

### Tarea 3.2: Nivel y cámara

**Archivos:**
- Crear: `src/juego/nivel.h/.c`, `src/juego/camara.h/.c`, `tests/prueba_camara.c`
- Modificar: `tests/Makefile` (añadir los `.c`), `tests/pruebas.c`

**Interfaces:**
- Produce: `nivel.h`: `#define ANCHO_NIVEL 512`, `#define ANCHO_PANTALLA 256`, `#define SUELO_Y 208`, `#define ANCHO_CELDA 64`, `u16 nivel_suelo_en(u16 x_pix);` (hoy devuelve `SUELO_Y`).
- Produce: `camara.h`: `u16 camara_para(u16 x_pix);` = `x_pix + 32 − 128` recortado a `[0, ANCHO_NIVEL − ANCHO_PANTALLA]` (§6).

- [ ] **Paso 1: Escribir `tests/prueba_camara.c`** con `void pruebas_camara(void)`: `camara_para(0) == 0`, `camara_para(96) == 0`, `camara_para(97) == 1`, `camara_para(200) == 104`, `camara_para(352) == 256`, `camara_para(448) == 256`, `nivel_suelo_en(0) == 208`, `nivel_suelo_en(500) == 208`.
- [ ] **Paso 2: Correr las pruebas** — Esperado: falla la compilación.
- [ ] **Paso 3: Implementar** (cálculo en `s16` para no desbordar `u16`).
- [ ] **Paso 4: Correr las pruebas** — Esperado: 0 fallos.
- [ ] **Paso 5: Compilar la ROM.** Esperado: `src/juego/nivel.obj` y `camara.obj` en el `.log`.
- [ ] **Paso 6: Commit** — `git commit -m "Nivel plano y camara con recorte, con pruebas"`

### Tarea 3.3: Mando (estructura), estados y tabla de animaciones

**Archivos:**
- Crear: `src/juego/mando.h` (solo la `struct`; `mando.c` llega en la Tarea 4.3), `src/juego/animaciones.h/.c`, `tests/tarou_anim.h` (copia fija para pruebas), `tests/prueba_animacion.c`

**Interfaces:**
- Produce: `mando.h`:
  ```c
  typedef struct {
      u8 izquierda, derecha, abajo, correr, saltar;                               /* mantenidos */
      u8 saltar_pulsado, atacar_pulsado, espada_pulsado, hud_pulsado, salir_pulsado; /* recién pulsados */
      u8 actividad;                                                               /* algo mantenido o pulsado */
  } Mando;
  void mando_leer(Mando *m);                                                      /* solo SNES */
  ```
- Produce: `animaciones.h`: `typedef enum { ESTADO_PARADO = 0, ESTADO_CAMINANDO, ESTADO_CORRIENDO, ESTADO_SALTANDO, ESTADO_CAYENDO, ESTADO_AGACHADO, ESTADO_AGACHADO_CAMINANDO, ESTADO_DESENVAINANDO, ESTADO_ENVAINANDO, ESTADO_ATACANDO, ESTADO_ECHANDOSE, ESTADO_DORMIDO, ESTADO_DESPERTANDO, ESTADO_CANTIDAD } Estado;`, `typedef struct { u8 cuadros, frames_por_cuadro, bucle; } AnimDef;`, `extern const AnimDef ANIMACIONES[ESTADO_CANTIDAD];`, `extern const char *const NOMBRES_ESTADO[ESTADO_CANTIDAD];` (los nombres sin prefijo, para el HUD).
- Produce: `animaciones.c` incluye `"tarou_anim.h"` (por ruta de inclusión: `tests/` en PC, `res/personajes/tarou/` en la ROM) y rellena la tabla con los frames por cuadro y bucles de §7.
- Produce: `tests/tarou_anim.h` con los mismos valores que los marcadores (Tarea 2.2, Paso 1).

- [ ] **Paso 1: Escribir `tests/prueba_animacion.c`** con `void pruebas_animacion(void)`: `ANIMACIONES[ESTADO_PARADO].cuadros == 4`, `ANIMACIONES[ESTADO_CAMINANDO].frames_por_cuadro == 6`, `ANIMACIONES[ESTADO_ATACANDO].bucle == 0`, `ANIMACIONES[ESTADO_DORMIDO].bucle == 1`, `NOMBRES_ESTADO[ESTADO_AGACHADO_CAMINANDO]` es `"AGACHADO_CAMINANDO"` (`strcmp == 0`).
- [ ] **Paso 2: Correr las pruebas** — Esperado: falla la compilación.
- [ ] **Paso 3: Implementar** los tres headers y `animaciones.c`.
- [ ] **Paso 4: Correr las pruebas** — Esperado: 0 fallos.
- [ ] **Paso 5: `make clean`, compilar la ROM.** Esperado: compila (`animaciones.c` encuentra `res/personajes/tarou/tarou_anim.h` por `-Ires/personajes/tarou`).
- [ ] **Paso 6: Commit** — `git commit -m "Estados del protagonista, tabla de animaciones y estructura del mando"`

### Tarea 3.4: Tarou — movimiento horizontal

**Archivos:**
- Crear: `src/juego/tarou.h/.c`, `tests/prueba_tarou.c`

**Interfaces:**
- Produce: `tarou.h`: las constantes de §5 (`VEL_CAMINAR_MAX 16`, `VEL_CORRER_MAX 32`, `VEL_AGACHADO_MAX 8`, `ACEL_SUELO 2`, `FRENADO_SUELO 2`, `ACEL_AIRE 1`, `GRAVEDAD 4`, `VEL_SALTO (-96)`, `VEL_SALTO_CORTE (-32)`, `VEL_TERMINAL 96`, `FRAMES_PARA_DORMIR 600`); `enum { ESPADA_ENVAINADA = 0, ESPADA_FUERA = 1 }`; `enum { DIR_DERECHA = 0, DIR_IZQUIERDA = 1 }`; `typedef struct { s16 x, y; u8 ancho, alto; } Caja;` y
  ```c
  typedef struct {
      s16 x_fp, y_fp, vx_fp, vy_fp;           /* mundo, 12.4 */
      u8 estado, espada, direccion, en_suelo;
      u8 cuadro, frames_cuadro, anim_terminada;
      u16 frames_quieto;
  } Tarou;
  void tarou_iniciar(Tarou *t, u16 x_pix, u16 suelo_y);          /* PARADO, envainada, derecha, pies en suelo */
  void tarou_actualizar(Tarou *t, const Mando *m, u16 suelo_y);  /* un frame: §5 orden */
  void tarou_caja_cuerpo(const Tarou *t, Caja *c);
  u8   tarou_caja_golpe(const Tarou *t, Caja *c);                /* 1 si hay golpe activo */
  ```
- Consume: `Mando`, `Estado`, `ANIMACIONES`, `PF`, `nivel.h`.

- [ ] **Paso 1: Escribir en `tests/prueba_tarou.c`** `void pruebas_tarou(void)` con estas pruebas (cada una crea `Tarou t; Mando m = {0}; tarou_iniciar(&t, 100, 208);` y usa un bucle `for` de N frames):
  - inicio: `t.estado == ESTADO_PARADO`, `t.x_fp == PF(100)`, `t.y_fp == PF(144)`, `t.espada == ESPADA_ENVAINADA`, `t.direccion == DIR_DERECHA`, `t.en_suelo == 1`.
  - `derecha` 1 frame → `ESTADO_CAMINANDO`, `vx_fp == 2`; 8 frames → `vx_fp == 16`; 20 frames → sigue 16 y `x_fp == PF(100) + 2+4+…` (sumar: 16·20 − (2+4+…+14) = 320 − 56 = 264).
  - `derecha` + `correr` 16 frames → `ESTADO_CORRIENDO`, `vx_fp == 32`; soltar `correr` → `ESTADO_CAMINANDO` y `vx_fp` baja 2 por frame hasta 16.
  - caminar 8 frames y soltar → en 8 frames `vx_fp == 0` y `ESTADO_PARADO`.
  - `izquierda` → `direccion == DIR_IZQUIERDA`, `vx_fp < 0`.
  - **Punto de atención 1:** `tarou_iniciar(&t, 0, 208)`, `izquierda` 30 frames → `x_fp == 0`, `vx_fp == 0` (el recorte anula la velocidad).
  - derecha desde 440 durante 60 frames → `x_fp == PF(448)`.
- [ ] **Paso 2: Correr las pruebas** — Esperado: falla la compilación.
- [ ] **Paso 3: Implementar `tarou_iniciar` y la parte horizontal de `tarou_actualizar`** (aceleración hacia la dirección, máximo según `correr`, frenado, integrar, recortar). Las demás ramas de la máquina de estados quedan como `default` que no hace nada todavía. `tarou_caja_cuerpo` según §5; `tarou_caja_golpe` devuelve 0.
- [ ] **Paso 4: Correr las pruebas** — Esperado: 0 fallos.
- [ ] **Paso 5: Compilar la ROM.** Esperado: `tarou.obj` compila con `tcc816` sin avisos.
- [ ] **Paso 6: Commit** — `git commit -m "Tarou: caminar, correr y frenar con punto fijo, con pruebas"`

### Tarea 3.5: Tarou — salto, gravedad y aterrizaje

**Archivos:**
- Modificar: `src/juego/tarou.c`, `tests/prueba_tarou.c`

- [ ] **Paso 1: Añadir pruebas:**
  - `saltar_pulsado` + `saltar` 1 frame → `ESTADO_SALTANDO`, `vy_fp == -96 + 4` (la gravedad se aplica el mismo frame), `en_suelo == 0`.
  - mantener `saltar` 24 frames → `vy_fp >= 0` en el frame 24 y `ESTADO_CAYENDO`; altura máxima: `PF_ENTERO(t.y_fp)` mínimo ≈ 144 − 69 = 75 (tolerancia ±2; la suma de `−96 + 4n` para n = 1..24 es −1104 fp).
  - **Punto de atención 2:** `saltar_pulsado` 1 frame y soltar en el frame 2 → `vy_fp == -32 + 4` en el frame 2; aterriza antes de 20 frames.
  - caída: `vy_fp` nunca supera 96 (dejar caer desde `y_fp = PF(0)` con `tarou_iniciar` y luego `t.y_fp = 0; t.en_suelo = 0; t.estado = ESTADO_CAYENDO;`).
  - aterrizaje: tras el salto completo `y_fp == PF(144)`, `vy_fp == 0`, `en_suelo == 1`, `estado == ESTADO_PARADO`; con `derecha` mantenida al aterrizar → `ESTADO_CAMINANDO`; con `abajo` → `ESTADO_AGACHADO`.
  - control aéreo: saltar sin dirección y pulsar `derecha` 10 frames en el aire → `vx_fp == 10`.
  - correr y saltar: `vx_fp` se conserva (32) durante el salto.
- [ ] **Paso 2: Correr las pruebas** — Esperado: fallan las nuevas.
- [ ] **Paso 3: Implementar** salto, `SALTANDO`, `CAYENDO`, corte, terminal y aterrizaje (§4, §5).
- [ ] **Paso 4: Correr las pruebas** — Esperado: 0 fallos.
- [ ] **Paso 5: Compilar la ROM.** Esperado: sin errores.
- [ ] **Paso 6: Commit** — `git commit -m "Tarou: salto de altura variable, gravedad y aterrizaje"`

### Tarea 3.6: Tarou — agacharse y espada

**Archivos:**
- Modificar: `src/juego/tarou.c`, `tests/prueba_tarou.c`

- [ ] **Paso 1: Añadir pruebas:**
  - `abajo` → `ESTADO_AGACHADO`; `tarou_caja_cuerpo` da `alto == 32` y `y == 144 + 32`; soltar → `ESTADO_PARADO`.
  - `abajo` + `derecha` 16 frames → `ESTADO_AGACHADO_CAMINANDO`, `vx_fp == 8`; con `correr` también 8.
  - agachado + `saltar_pulsado` → `ESTADO_SALTANDO`.
  - `espada_pulsado` en `PARADO` → `ESTADO_DESENVAINANDO`; mantener `derecha` durante la animación no mueve (`vx_fp == 0`); tras `ANIMACIONES[ESTADO_DESENVAINANDO].cuadros * frames_por_cuadro` frames → `ESTADO_PARADO` y `espada == ESPADA_FUERA`.
  - `espada_pulsado` con espada fuera → `ESTADO_ENVAINANDO` → al final `ESPADA_ENVAINADA`.
  - `atacar_pulsado` con espada envainada → sigue `PARADO`; con espada fuera → `ESTADO_ATACANDO`, `vx_fp == 0`, `tarou_caja_golpe` devuelve 1 con `ancho == 28`, `alto == 16`, `x == x_pix + 44` mirando a la derecha y `x == x_pix + 64 − 72` mirando a la izquierda; al terminar → `PARADO` y `tarou_caja_golpe` devuelve 0.
  - `atacar_pulsado` en el aire → no cambia de estado; agachado → no cambia.
  - **Punto de atención 3:** `espada_pulsado` y `atacar_pulsado` el mismo frame con espada fuera → `ESTADO_ENVAINANDO`.
- [ ] **Paso 2: Correr las pruebas** — Esperado: fallan las nuevas.
- [ ] **Paso 3: Implementar** los estados `AGACHADO`, `AGACHADO_CAMINANDO`, `DESENVAINANDO`, `ENVAINANDO`, `ATACANDO`, las cajas y la prioridad de §4.
- [ ] **Paso 4: Correr las pruebas** — Esperado: 0 fallos.
- [ ] **Paso 5: Compilar la ROM.**
- [ ] **Paso 6: Commit** — `git commit -m "Tarou: agacharse, caminar agachado, envainar, desenvainar y atacar"`

### Tarea 3.7: Tarou — dormir y despertar; animación

**Archivos:**
- Modificar: `src/juego/tarou.c`, `tests/prueba_tarou.c`

- [ ] **Paso 1: Añadir pruebas:**
  - 599 frames sin actividad → `PARADO`; 600 → `ESTADO_ECHANDOSE`; al terminar su animación → `ESTADO_DORMIDO`; `cuadro` recorre 0..3 en bucle.
  - en `DORMIDO`, `m.actividad = 1` 1 frame → `ESTADO_DESPERTANDO`; con `derecha` mantenida durante la animación `vx_fp == 0`; al terminar → `PARADO`, `frames_quieto == 0`.
  - **Punto de atención 4:** actividad en el frame 2 de `ECHANDOSE` → `ESTADO_DESPERTANDO` directo; nunca `DORMIDO`.
  - caminar 100 frames y soltar → `frames_quieto` empieza a contar solo al llegar a `PARADO`.
  - animación: en `CAMINANDO`, `cuadro` cambia cada 6 frames y vuelve a 0 tras el último; en `ATACANDO` (sin bucle), `cuadro` se queda en el último y `anim_terminada == 1`; cambiar de estado pone `cuadro == 0` y `frames_cuadro == 0`.
- [ ] **Paso 2: Correr las pruebas** — Esperado: fallan las nuevas.
- [ ] **Paso 3: Implementar** `ECHANDOSE`, `DORMIDO`, `DESPERTANDO`, el contador de sueño y el avance de animación común a todos los estados.
- [ ] **Paso 4: Correr las pruebas** — Esperado: 0 fallos.
- [ ] **Paso 5: Compilar la ROM.**
- [ ] **Paso 6: Commit** — `git commit -m "Tarou: dormir por inactividad, despertar y avance de animacion"`

---

## Fase 4 — El nivel en la SNES

### Tarea 4.1: `pantalla_nivel1` con fondos y cámara fija

**Archivos:**
- Crear: `src/pantallas/pantalla_nivel1.c`
- Borrar: `src/pantallas/pantalla_proximamente.c`
- Modificar: `src/sistema/pantallas.h` (`PANTALLA_PROXIMAMENTE` → `PANTALLA_NIVEL1`; prototipos `pantalla_nivel1_iniciar/actualizar/vblank`), `src/sistema/secuencia.c` (tres `switch`), `src/pantallas/pantalla_titulo.c` (Start → `PANTALLA_NIVEL1`), `docs/diseno-secuencia-arranque.md` (pantalla 7)

**Interfaces:**
- Produce: `pantalla_nivel1_iniciar()`: `setMode(BG_MODE1, BG3_MODE1_PRIORITY_HIGH)`, `fondo16_cargar` de `cerca_*` (BG0, paleta 1, `VRAM_FONDO_A`/`VRAM_MAPA_A`, `SC_64x32`) y de `nivel1_lejos_*` (BG1, paleta 2, `VRAM_FONDO_B`/`VRAM_MAPA_B`, `SC_64x32`), `fuente_2bpp(2)` + `bgSetDisable(2)`, HDMA del cielo, `camara = 0`, `fade_in`.
- Produce: `pantalla_nivel1_actualizar()`: START → `salir()` (fade_out, `setModeHdmaReset(0)`, `secuencia_cambiar(PANTALLA_TITULO)`); de momento Izquierda/Derecha mueven `camara` ±2 por frame para probar el scroll; `bgSetScroll(0, camara, 0)`, `bgSetScroll(1, camara >> 1, 0)`.
- Produce: `pantalla_nivel1_vblank()` vacía por ahora.

- [ ] **Paso 1: Escribir los archivos** con la cabecera §12.
- [ ] **Paso 2: `make clean`, compilar la ROM.**
- [ ] **Paso 3: Mesen:** Start en el título entra al nivel; las dos capas se ven; Izquierda/Derecha desplazan con parallax; la cámara se detiene en 0 y 256; START vuelve al título y el título se ve bien (HDMA, sprites).
- [ ] **Paso 4: Capturas** — `Mesen.exe ProyectoSNES.sfc scripts/capturas_mesen.lua` y revisar una captura del nivel.
- [ ] **Paso 5: Commit** — `git commit -m "Pantalla del nivel 1 con las dos capas, parallax y cielo"`

### Tarea 4.2: Sprite dinámico de Tarou

**Archivos:**
- Crear: `src/juego/tarou_sprite.h/.c`
- Modificar: `src/pantallas/pantalla_nivel1.c` (Tarou en reposo en `x = 64`)

**Interfaces:**
- Produce: `tarou_sprite.h`:
  ```c
  void tarou_sprite_cargar(void);                           /* oamInitDynamicSprite + paleta 0 */
  void tarou_sprite_dibujar(const Tarou *t, u16 camara);    /* oamMetaDrawDyn32 con espejo según direccion */
  void tarou_sprite_ocultar(void);
  const t_metasprite *tarou_sprite_tabla(u8 estado, u8 cuadro);  /* tabla del cuadro, mirando a la derecha */
  u8 *tarou_sprite_tiles(u8 estado);                        /* &tarou_<anim>_til */
  ```
- Consume: las 13 `_meta.inc` (incluidas **solo** en `tarou_sprite.c`), las etiquetas de `data.asm` (Tarea 2.3), `Tarou`, `ANIMACIONES`.

- [ ] **Paso 1: Escribir `tarou_sprite.c`**: tabla estática `const t_metasprite *const *TABLAS[ESTADO_CANTIDAD]` y `u8 *const TILES[ESTADO_CANTIDAD]` indexadas por estado; `espejo[5]` en RAM construido como dice §7; `tarou_sprite_dibujar` calcula `x_pantalla = PF_ENTERO(x_fp) − camara`, `y_pantalla = PF_ENTERO(y_fp)` y llama a `oamMetaDrawDyn32(0, …)`.
- [ ] **Paso 2: En `pantalla_nivel1`:** `tarou_sprite_cargar()` al iniciar, `tarou_iniciar(&tarou, 64, nivel_suelo_en(96))`, cada frame `tarou_sprite_dibujar` + `oamInitDynamicSpriteEndFrame()` al final de `actualizar`; `oamVramQueueUpdate()` en `vblank`; `tarou_sprite_ocultar()` en `salir()`.
- [ ] **Paso 3: `make clean`, compilar la ROM.**
- [ ] **Paso 4: Mesen:** el marcador «reposo 1» se ve entero (4 piezas en su sitio), con los pies en la fila 208 (ajustar `SUELO_Y` si no coincide con el suelo del fondo y anotar el valor medido en §5). Forzar `t.direccion = DIR_IZQUIERDA` un frame y comprobar el espejo.
- [ ] **Paso 5: Commit** — `git commit -m "Tarou en el nivel como metasprite dinamico de 64x64"`

### Tarea 4.3: Mando, física y cámara en marcha

**Archivos:**
- Crear: `src/juego/mando.c`
- Modificar: `src/pantallas/pantalla_nivel1.c`

**Interfaces:**
- Produce: `mando_leer(Mando *m)`: rellena desde `padsCurrent(0)` (`KEY_LEFT`, `KEY_RIGHT`, `KEY_DOWN`, `KEY_Y`, `KEY_B`) y `padsDown(0)` (`KEY_B`, `KEY_A`, `KEY_R`, `KEY_SELECT`, `KEY_START`); `actividad = (padsCurrent(0) | padsDown(0)) != 0`.

- [ ] **Paso 1: Escribir `mando.c`** y conectar el flujo de §11 en `pantalla_nivel1_actualizar` (quitar el movimiento de prueba de la cámara).
- [ ] **Paso 2: `make clean`, compilar la ROM.**
- [ ] **Paso 3: Mesen** (lista §13.3, puntos 2 y 3): caminar, correr, saltar largo y corto, control aéreo, cámara, bordes. Anotar en §5 cualquier constante que se cambie.
- [ ] **Paso 4: Commit** — `git commit -m "Tarou se mueve con el mando: caminar, correr, saltar y camara"`

### Tarea 4.4: HUD de depuración

**Archivos:**
- Crear: `src/juego/hud_debug.h/.c`
- Modificar: `src/pantallas/pantalla_nivel1.c`

**Interfaces:**
- Produce: `void hud_debug_iniciar(void);` (apagado, texto limpio), `void hud_debug_alternar(void);` (`bgSetEnable(2)` / `bgSetDisable(2)` + `limpiar_texto()`), `void hud_debug_dibujar(const Tarou *t, u16 camara);` (solo si está visible; las seis filas de §9 con `consoleDrawText(1, fila, "X %d  VX %d   ", …)`; los espacios finales borran los dígitos viejos).

- [ ] **Paso 1: Escribir los archivos** y conectar SELECT.
- [ ] **Paso 2: `make clean`, compilar la ROM.**
- [ ] **Paso 3: Mesen:** SELECT muestra y oculta el HUD; `VX` negativo se imprime con signo (si `%d` no lo soporta, imprimir el signo a mano y anotarlo en §15); los valores coinciden con lo esperado (velocidad 16 caminando, 32 corriendo, `CAM` 0..256).
- [ ] **Paso 4: Commit** — `git commit -m "HUD de depuracion en BG3 alternado con SELECT"`

### Tarea 4.5: Todas las mecánicas en pantalla y afinado

**Archivos:**
- Modificar: `src/juego/tarou.h` (constantes afinadas), `docs/diseno-demo-jugable.md` (§5, §7 valores finales), `scripts/capturas_mesen.lua` (si hace falta más tiempo de captura)

- [ ] **Paso 1: Mesen** (lista §13.3 completa): agacharse, caminar agachado, saltar agachado, R, A con y sin espada, caja de golpe en el HUD, dormir a los 10 s, despertar. Anotar fallos.
- [ ] **Paso 2: Corregir y afinar** constantes; cada cambio de constante se refleja en las pruebas de PC (correr las pruebas) y en §5.
- [ ] **Paso 3: Capturas** de cada mecánica guardadas en `docs/screenshots/revision-1/`.
- [ ] **Paso 4: Commit** — `git commit -m "Revision 1 jugable: todas las mecanicas verificadas en Mesen"`

---

## Fase 5 — Intro, título y esqueleto

### Tarea 5.1: Tarou nuevo en intro y título

**Archivos:**
- Modificar: `src/pantallas/personajes.c/.h`, `src/pantallas/pantalla_intro.c`, `src/pantallas/pantalla_titulo.c`, `data.asm` (el lobo deja de incluirse en la ROM; conservar el PNG y la regla del Makefile para la Etapa 3 con un comentario)

**Interfaces:**
- Produce: `personajes_cargar()` copia `tarou_reposo_til` (8 KB) a VRAM 0x0000 por DMA y `tarou_pal` a la paleta 0; Bárbara a continuación como hasta ahora. `OBJ_SIZE32_L64`.
- Produce: `static u16 tile_del_bloque32(u8 n)` = `(n / 4) * 64 + (n % 4) * 4`; `dibujar()` recorre la tabla `tarou_sprite_tabla(ESTADO_PARADO, cuadro)` y escribe 4 `oamSet`/`oamSetEx(OBJ_SMALL)` (ids 0..3 → `0, 4, 8, 12`), con `dx` intercambiado y `hflip` cuando mira a la izquierda. Bárbara pasa al id 4 (`16`).
- Firma nueva: `void personajes_mostrar(u16 xt, u16 yt, u8 tarou_mira_izquierda, u16 xb, u16 yb);`

- [ ] **Paso 1: Reescribir `personajes.c`** y ajustar las dos llamadas (`pantalla_intro.c` escena 3, `pantalla_titulo.c`).
- [ ] **Paso 2: `make clean`, compilar la ROM.** Esperado: el `.log` ya no lista `.rodata_lobo`.
- [ ] **Paso 3: Mesen:** en la escena 3 y en el título se ve el marcador «reposo» entero con los 4 cuadros en bucle junto a Bárbara; el lobo no aparece. Si las piezas salen desordenadas, corregir `tile_del_bloque32` con la numeración real anotada en la Tarea 2.3 y actualizar §15.
- [ ] **Paso 4: Commit** — `git commit -m "Intro y titulo muestran al nuevo Tarou"`

### Tarea 5.2: Esqueleto de stubs

**Archivos:**
- Crear: `src/juego/barbara.h/.c`, `src/juego/cambio_protagonista.h/.c`, `src/enemigos/enemigos.h/.c`, `src/objetos/objetos.h/.c`, `src/hud/hud.h/.c`, `src/audio/audio.h/.c`, `src/niveles/niveles.h/.c`

**Interfaces:**
- Produce, por stub, el contrato previsto como prototipos comentados (no implementados) y una función `void <modulo>_pendiente(void) {}` para que el archivo compile. Contratos:
  - `barbara.h`: igual que `tarou.h` con `Barbara` (Revisión 2).
  - `cambio_protagonista.h`: `protagonista_activo()`, `protagonista_cambiar()` (Etapa 4).
  - `enemigos.h`: `enemigos_cargar(nivel)`, `enemigos_actualizar()`, `enemigos_dibujar(camara)`, `enemigos_golpe(const Caja *)` (Etapa 3; el lobo samurái como primer enemigo).
  - `objetos.h`: `objetos_cargar(nivel)`, `objetos_actualizar()`, `objetos_recoger(const Caja *)` (Etapa 3).
  - `hud.h`: `hud_cargar()`, `hud_dibujar(vida, objetos)` (Etapa 3).
  - `audio.h`: `audio_iniciar()`, `audio_musica(id)`, `audio_efecto(id)`, `audio_procesar()` (sesión de audio; `spcBoot/spcLoad/spcProcess`).
  - `niveles.h`: `nivel_cargar(id)` con tilemap y colisión por tiles, `nivel_scroll(camara)` (Etapa 2/3; motor de mapas de LikeMario).

- [ ] **Paso 1: Escribir los 14 archivos** con la cabecera §12 y `PENDIENTE: Etapa N — …`.
- [ ] **Paso 2: Compilar la ROM.** Esperado: los 7 `.obj` nuevos en el `.log`; tamaño de código sin cambios apreciables.
- [ ] **Paso 3: Commit** — `git commit -m "Esqueleto de subsistemas futuros: stubs documentados"`

---

## Fase 6 — Documentación

### Tarea 6.1: Mapa de archivos

**Archivos:**
- Crear: `docs/mapa-de-archivos.md`

- [ ] **Paso 1: Escribir** una ficha por archivo del repositorio (raíz: `Makefile`, `data.asm`, `run.bat`, `.gitignore`, `.clang-format`, `CLAUDE.md`, `hdr.asm`/`linkfile` generados, `ProyectoSNES.sfc/.sym/.log`; `src/**`; `res/**` por carpeta; `scripts/*`; `tests/*`; `docs/*`; `.vscode/*`; `tools/` como caja negra) con: qué hace, qué incluye / quién lo usa, hardware que toca, y un diagrama ASCII de inclusión de `src/`.
- [ ] **Paso 2: Verificar** que cada archivo de `git ls-files` tiene ficha (script de una línea: comparar la lista con los nombres citados).
- [ ] **Paso 3: Commit** — `git commit -m "Documentacion: mapa de archivos del proyecto"`

### Tarea 6.2: Tubería de compilación y Makefile

**Archivos:**
- Crear: `docs/tuberia-de-compilacion.md`

- [ ] **Paso 1: Escribir**: diagrama ASCII `.c → 816-tcc → .ps → 816-opt → .asm → wla-65816 → .obj → wlalink (linkfile, hdr.asm) → .sfc + .sym`; `gfx4snes` (`.png → .pic/.pal/.map/_meta.inc`) y por qué `bitmaps` va antes; el Makefile regla por regla (cada variable y cada regla con una frase); `snes_rules` (qué hace, qué no rastrea); `run.bat` y las tareas de VS Code; la cabecera de la ROM (LoROM, SlowROM, 16 bancos, `ROMSIZE`); lectura del `.log` (bancos y RAM libres); `tests/Makefile` y gcc.
- [ ] **Paso 2: Verificar** cada comando citado ejecutándolo una vez.
- [ ] **Paso 3: Commit** — `git commit -m "Documentacion: tuberia de compilacion y Makefile"`

### Tarea 6.3: Mesen y depuración

**Archivos:**
- Crear: `docs/mesen-y-depuracion.md`

- [ ] **Paso 1: Escribir**: cómo Mesen carga el `.sfc`; cargar el `.sym` de WLA en el depurador (Debug → Debugger → File → Import labels; verificar el menú real y anotarlo); mapeo de mando (Options → Input); el HUD de depuración; el script Lua de capturas y el ajuste de acceso a E/S; visores de VRAM, OAM, paletas y tilemaps para comprobar sprites y fondos; breakpoints en funciones C por su etiqueta.
- [ ] **Paso 2: Verificar** cada ruta de menú abriendo Mesen.
- [ ] **Paso 3: Commit** — `git commit -m "Documentacion: Mesen y depuracion"`

### Tarea 6.4: Hardware por módulo

**Archivos:**
- Crear: `docs/hardware-snes-por-modulo.md`

- [ ] **Paso 1: Escribir**: tabla módulo → chip y registros (CPU 65816 y RAM para la lógica pura; PPU: modos, BG, scroll, OAM, CGRAM, VRAM, HDMA, brillo; mando: puertos de joypad leídos por el NMI del SDK; SPC700: solo en el stub de audio), con enlace a la función del SDK que los toca; el mapa de VRAM de `comun.h` como diagrama; el mapa de CGRAM (paletas de fondo 0..7 y de sprites 0..7 y quién usa cada una); el mapa de ROM por bancos (qué sección va en cuál, del `.log`).
- [ ] **Paso 2: Commit** — `git commit -m "Documentacion: hardware de la SNES por modulo"`

### Tarea 6.5: Índice y actualización de los documentos existentes

**Archivos:**
- Crear: `docs/README.md` (índice de todos los documentos con una frase cada uno)
- Modificar: `docs/diseno-secuencia-arranque.md` (pantalla 7 → nivel 1; sprites nuevos), `docs/plan-desarrollo.md` (Etapa 2: marcar los ítems cubiertos), `docs/glosario-snes.md` (términos nuevos que hayan aparecido), `CLAUDE.md` (secciones Architecture y «What the current ROM does»; comandos de pruebas en PC)

- [ ] **Paso 1: Escribir y actualizar.**
- [ ] **Paso 2: Verificar** que `CLAUDE.md` no cita archivos que ya no existen (`git grep` de cada ruta citada).
- [ ] **Paso 3: Commit** — `git commit -m "Documentacion: indice, secuencia de arranque, plan de desarrollo y CLAUDE.md al dia"`

---

## Fase 7 — Plan de aprendizaje

### Tarea 7.1: `aprendizaje/PLAN.md`

**Archivos:**
- Crear: `aprendizaje/PLAN.md`

- [ ] **Paso 1: Escribir el plan** con, para cada unidad: objetivo, conceptos (cada uno definido en una frase o enlazado al glosario), archivos que tendrá (`README.md` de teoría, `demo_*.c`, `ejercicio_*.c`, `solucion/`, y para las mini-ROMs `rom/Makefile`), qué código del juego explica, prerequisitos, tipo de práctica (PC con gcc, mini-ROM, ejercicio sobre el juego real) y criterio de aprobación. Unidades, en este orden:
  - 00 C en PC: lógica, funciones, arrays (lo que ya existe en `etapa0`) + `struct`, punteros y `#define`.
  - 01 Enteros de 16 bits y punto fijo (`puntofijo.h`, `tests/prueba_puntofijo.c`).
  - 02 Máquinas de estados (`secuencia.c`, `Estado` de `tarou.c`).
  - 03 De `.c` a `.sfc`: la tubería de compilación y leer el `.asm` generado (mini-ROM «hola mundo»).
  - 04 Frame, VBlank, NMI y el bucle principal (`main.c`; mini-ROM con contador).
  - 05 El mando (`mando.c`; mini-ROM que muestra los botones).
  - 06 Tiles, paletas, tilemaps y fondos (`comun.c`; mini-ROM con un fondo).
  - 07 Scroll, cámara y parallax (`camara.c`, `pantalla_nivel1.c`; mini-ROM).
  - 08 Sprites estáticos y OAM (`personajes.c`; mini-ROM).
  - 09 Metasprites y sprites dinámicos (`tarou_sprite.c`; mini-ROM).
  - 10 Física de plataformas: velocidad, aceleración, gravedad, salto (`tarou.c`, pruebas en PC, afinado con el HUD).
  - 11 Animación por estados (`animaciones.c`, `tarou.c`).
  - 12 HDMA y efectos de la PPU (cielo del título; mini-ROM).
  - 13 Memoria: VRAM, CGRAM, OAM, RAM y ROM por bancos; `data.asm`; leer el `.log` y el `.sym`.
  - 14 Ensamblador 65816 I: registros, modos de direccionamiento, leer el `.asm` del compilador y del SDK.
  - 15 Ensamblador 65816 II: escribir una rutina en `.asm` llamable desde C (mini-ROM).
  - 16 Mesen: depurador, breakpoints, visores de VRAM/OAM (ejercicios sobre el juego real).
  - 17 SPC700 y audio: el procesador, su ensamblador, el driver del SDK y `smconv`; mini-ROM que reproduce un `.it` de LikeMario (sin componer).
  - 18 Herramientas: `gfx4snes`, los scripts Python del pipeline y el Makefile.
- [ ] **Paso 2: Verificar** que cada ítem de «Ítems técnicos a estudiar» de las Etapas 0–2 de `docs/plan-desarrollo.md` aparece en alguna unidad.
- [ ] **Paso 3: Commit** — `git commit -m "Plan de la carpeta aprendizaje: 19 unidades de C y ensamblador para SNES"`

---

## Autorrevisión del plan (hecha al escribirlo)

- **Cobertura de la spec:** §2 decisiones 1–20 → Fases 0–5; §3 mando → 4.3; §4 estados → 3.4–3.7; §5 física → 3.4–3.5; §6 cámara → 3.2; §7 sprite → 2.3, 4.2; §8 fondo → 2.4, 4.1; §9 HUD → 4.4; §10 pipeline → 2.1–2.2; §11 arquitectura → 1.1, 1.3, 5.2; §12 cabecera → 1.3 y cada tarea; §13 verificación → pasos de Mesen y pruebas; documentación → Fase 6; aprendizaje → Fase 7.
- **Tipos y nombres:** `Tarou`, `Mando`, `Caja`, `Estado`, `AnimDef`, `ANIMACIONES`, `NOMBRES_ESTADO`, `PF`, `PF_ENTERO`, `camara_para`, `nivel_suelo_en`, `tarou_sprite_tabla` se definen antes de usarse y con la misma firma en todas las tareas.
- **Puntos de atención:** los cinco tienen prueba (3.4, 3.5, 3.6, 3.7, 2.2).
