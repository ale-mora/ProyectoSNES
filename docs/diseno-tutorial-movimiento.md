# Diseño — Nivel 1 como tutorial de movimiento

**Proyecto:** _Tarou: A Dog's Journey_ · **Etapa del plan:** 2 (Prototipo) ·
**Revisión:** 2 · **Fecha de esta versión:** 2026-10-20 · **Estado:** diseño
acordado en sesión de preguntas; fase 1 (prueba del motor de mapas) hecha.

El nivel 1 deja de ser un suelo plano de 512 px y pasa a ser un **tutorial**:
un recorrido de 8 pantallas (2048 px) en el que cada obstáculo enseña una
acción de Tarou, con carteles dibujados en el escenario, hasta una salida que
lleva al siguiente nivel. Parte de la Revisión 1 (`docs/diseno-demo-jugable.md`),
cuyas mecánicas no cambian: aquí se añade el **escenario con colisión**.

> **Vocabulario.** Los términos son los de `docs/glosario-snes.md`. Nuevos en
> este documento: *metatile*, *AABB*, *punto de control*, *plataforma de un
> solo sentido*, *streaming de columnas* (se añaden al glosario en la fase 8).

## 0. Historial

| Versión | Fecha | Qué decía |
|---|---|---|
| v1 | 2026-10-20 | Diseño acordado y resultado de la prueba del motor de mapas (§5). |

## 1. Decisiones

**Del autor** (sesión del 2026-10-20):

| # | Tema | Decisión |
|---|---|---|
| 1 | Qué enseña | Todo el repertorio: un obstáculo por acción. |
| 2 | Final | Salida al siguiente nivel. |
| 3 | Cómo enseña | Carteles dibujados en el escenario, junto a cada obstáculo. |
| 4 | Fallar | Fosos y algo que hace daño, con reaparición. |
| 5 | Arte | Se reutiliza el tileset *Sidescroller* del suelo; se pide a Pixellab solo lo que falte. |
| 6 | Longitud | Unos 2048 px (8 pantallas), con el fondo dibujándose por columnas. |
| 7 | Editor | Tiled (lo instala el autor: `winget install thorbjorn.Tiled`). |

**De Claude** (el autor puede corregirlas):

| # | Tema | Decisión |
|---|---|---|
| C1 | Siguiente nivel | No existe nivel 2: una pantalla nueva `PANTALLA_PROXIMAMENTE` muestra «LEVEL 2 / COMING SOON» y vuelve al título. |
| C2 | Daño | Sin vida ni animación de herida: pinchos o foso → fundido y reaparición en el último punto de control (uno por cartel). |
| C3 | Carteles | Tablero de Pixellab (provisional: marcador generado); las letras las compone un script con la fuente de consola. |
| C4 | Rompible | Un tronco como **sprite** con su caja, sólido hasta que lo toca la caja `GOLPE` (el motor de mapas lee el mapa desde la ROM: no se pueden cambiar tiles, §5). |
| C5 | Motor | El motor de mapas de PVSnesLib (`mapLoad`/`mapUpdate`/`mapVblank`) dibuja la capa cercana por columnas; la **colisión** la resuelve un módulo propio de lógica pura, probado en el PC, sobre una rejilla generada desde el mismo mapa de Tiled. |

## 2. El recorrido

Dimensionado con la física real (`src/juego/tarou.h`): salto de unos 69 px y
unos 48 cuadros en el aire; caminar 1 px/cuadro, correr 2 px/cuadro; cuerpo de
52×50 px de pie y 36 px agachado.

| # | Pantalla | Cartel | Obstáculo | Qué se exige |
|---|---|---|---|---|
| 1 | 0 | `← → WALK` | Llano | — |
| 2 | 1 | `B: JUMP` | Escalón de 16 px y luego uno de 32 px | — |
| 3 | 2 | `HOLD B: JUMP HIGHER` | Muro de 56 px | un salto corto no llega |
| 4 | 3 | `DOWN: CROUCH` | Túnel de 40 px de alto y 96 px de largo | de pie no cabe; bajo el techo, soltar Abajo **no** levanta a Tarou |
| 5 | 4 | `HOLD Y: RUN` | Foso de unos 80 px | caminando no se cruza; corriendo sí |
| 6 | 5 | `MIND THE THORNS` | Zarzas (`T_SPIKE`) con plataformas de un solo sentido (`T_PLATE`) encima | tocar zarza = reaparecer |
| 7 | 6 | `R: SWORD  A: ATTACK` | Tronco rompible que bloquea el paso | sin espada fuera, el tronco no se rompe |
| 8 | 7 | `EXIT →` | Salida | lleva a `PANTALLA_PROXIMAMENTE` |

## 3. Tubería de datos

```
Pixellab (Sidescroller) ─┐
seto, carteles, zarzas ──┴─► scripts/preparar_tileset_nivel.py ─► res/nivel1/tileset.png + tileset.tsx
                                                                     │
                         Tiled (el autor) ◄──────────────────────────┘
                              │ res/nivel1/tutorial.tmj
             ┌────────────────┴───────────────────┐
     tmx2snes (SDK): .m16 .b16 .t16        scripts/generar_colision.py:
     → motor de mapas (dibujo)             rejilla de colisión + objetos (.h y .bin)
                                           → src/juego/colision.c (lógica pura)
```

Tiled es la **única fuente** del nivel: el dibujo y la colisión salen del mismo
`.tmj`. Cada tile del tileset lleva su propiedad `attribute` (`FF00` sólido,
`0004` pincho, `0008` plataforma, `0` vacío), como en
`tools/pvsneslib/snes-examples/graphics/maps/slopemario/map_1_1.tmj`.

## 4. Reparto de la VRAM nuevo

El motor de mapas escribe la capa cercana **siempre en 0x6800** (64×32). Las
constantes de `src/sistema/comun.h` pasan a:

| Zona | Antes | Después |
|---|---|---|
| `VRAM_MAPA_A` (capa cercana, 64×32) | 0x6C00–0x73FF | **0x6800–0x6FFF** |
| `VRAM_MAPA_B` (capa lejana, 64×32) | 0x7400–0x7BFF (el comentario decía 0x77FF) | **0x7000–0x77FF** |
| `VRAM_MAPA_TEXTO` (texto, 32×32) | 0x6800–0x6BFF | **0x7800–0x7BFF** |

## 5. Fase 1: prueba del motor de mapas (2026-10-20)

Mini-ROM hecho con la receta de `aprendizaje/PLAN.md` §6.3 a partir de
`snes-examples/graphics/maps/tiled`, con el mapa ensanchado a **256×28 tiles**
(2048×224 px), la capa cercana en 0x6800 (SC_64x32) y una capa de texto en
BG3 con su mapa en 0x7800. Un «jugador» falso recorre el mapa a 2 px/cuadro y
`mapUpdateCamera` lo sigue. Capturas con `scripts/capturar_mesen.ps1 -Rom`.

| Pregunta | Resultado |
|---|---|
| ¿Conviven el mapa en 0x6800 y el texto en 0x7800? | **Sí.** El mapa se dibuja sin corrupción y el texto aparece encima. |
| ¿Se desplaza un mapa de 2048 px? | **Sí**, a 2 px/cuadro, sin costuras visibles (el ejemplo `slopemario` usa además 300 tiles de ancho). El `.m16` mide 14 342 bytes (256×28×2 + 6 de cabecera): cabe holgado en los bancos 14–15, hoy libres. |
| ¿Límite de tiles? | El índice de tile del mapa es de 10 bits (1024), pero nuestro presupuesto real es la zona `VRAM_FONDO_A`: **512 tiles de 4 bpp**. El `.b16`/`.t16` miden 2 bytes por tile del tileset. |
| ¿Cámara? | `x_pos` es el borde izquierdo de la cámara; los sprites se dibujan en `x_mundo − x_pos` (como LikeMario). Su zona muerta y que la cámara llegue al final se miden en la fase 2 con el HUD de depuración: el texto de la prueba salió con los caracteres desplazados (fuente por defecto del SDK) y no sirvió para leer números. |
| ¿Se pueden cambiar tiles en marcha? | **No por la API** (`map.h` no tiene ninguna función de escritura; `mapLoad` recibe el mapa en ROM). Por eso el tronco es un sprite (C4). |

**Trampa encontrada:** `tmx2snes` lee el JSON de Tiled con *cute_tiled*, que
falla («Invalid integer found during parse») si hay un **espacio después de
los dos puntos** (`"width": 256`). Tiled exporta sin espacios; un script que
escriba `.tmj` debe usar `json.dump(..., separators=(",", ":"))`.

## 6. Fases

Un commit por fase, sin push. Detalle en el plan aprobado:

1. Prueba del motor de mapas (hecha, §5).
2. Tileset, Tiled y motor en el nivel con un mapa plano de 2048 px; nuevo reparto de VRAM.
3. Colisión con el escenario (lógica pura, `src/juego/colision.c`, pruebas en el PC).
4. Fosos, zarzas y reaparición en puntos de control.
5. Tronco rompible (`src/objetos/`).
6. Carteles.
7. Salida y pantalla `PANTALLA_PROXIMAMENTE`.
8. Documentación (glosario, mapa de archivos, tubería, plan de desarrollo, `CLAUDE.md`, `aprendizaje/PLAN.md`).

## 7. Verificación

- Pruebas en el PC: `colision`, Tarou adaptado y un **recorrido completo**
  guionizado sobre la rejilla del nivel real que llega a la salida; y pruebas
  negativas: el foso no se cruza caminando, el muro no se salta con un salto
  corto, el túnel no se pasa de pie, el tronco bloquea hasta el golpe.
- Pruebas de los scripts con `unittest`.
- `make clean` + `make` sin avisos.
- Mesen con `scripts/capturar_mesen.ps1`, una captura por obstáculo, a 60 FPS.
- El autor juega el tutorial y retoca el mapa en Tiled.
