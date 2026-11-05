# Unidad 27 — Un fondo hecho a mano: la aldea, el primer plano que tapa y los sprites del decorado

> **Código real, rama principal.** Esta unidad enseña sobre el fondo de la fase 1 (commit
> `6845b5f`, 2026-11-05): `scripts/preparar_aldea.py` (el conversor), `res/aldea/` (lo que
> escribe, versionado), `src/juego/fondo_nivel.c` (la carga y el noren),
> `src/pantallas/pantalla_fase1.c` (el VBlank), `src/juego/ambiente.c` y
> `ambiente_sprite.c` (el humo y los pájaros). La demo lee `res/aldea/` desde esta carpeta;
> la parte B del ejercicio se hace en un clon:
>
> ```text
> git clone -q E:\ProyectoSNES E:\ProyectoSNES_trabajo\u27
> ```
>
> **No hay ficha en `PLAN.md` para esta unidad**: se añadió el 2026-11-05, cuando el código
> ya existía, así que no promete nada que falte. Lo que **no existe** todavía (bosque,
> monasterio y dojo; las franjas de parallax por HDMA; objetos rompibles) va en «Por venir».

**Objetivo:** entender cómo se convierte un dibujo grande en lo que la SNES sabe pintar —
**tiles** de 8×8 y un **mapa** que dice qué tile va en cada celda — sin pasarse de la VRAM,
y qué decisiones hay detrás: por qué las piezas van en múltiplos de 8, por qué un tile
volteado no cuesta nada, cómo se usan **dos paletas** en una sola capa, qué se hace cuando
salen más de 512 tiles, cómo un poste del fondo **tapa** a un personaje, cómo se **anima** un
trozo del fondo cambiando sus tiles, y cómo se pone en el cielo un adorno que se mueve
(humo y pájaros) sin tocar el fondo.

**Prerrequisitos:** 06 (tiles, paletas y mapas: el formato 4bpp, la palabra del mapa), 07
(scroll y el anillo de columnas), 12 (HDMA: el degradado del cielo), 18 (gfx4snes y los
scripts de Python con pruebas), 19 (el arte de Pixellab), 21 (capa de sprites y presupuesto
del VBlank), 22 (el carril y la prioridad), 25 (paletas 4–7 translúcidas).
**Práctica:** PC (Python con pruebas) + juego real en un clon, con capturas de Mesen.
**Tiempo orientativo:** dos tardes.

## Conceptos

### Por qué un conversor propio

Hasta la aldea, todos los fondos se convertían con **gfx4snes** (unidad 18): se le da un PNG
y devuelve tiles, mapa y paleta. Para la aldea hacía falta algo que gfx4snes no hace:

- poner el **bit de prioridad** en algunas celdas del mapa (el primer plano);
- usar **dos paletas** en la misma capa y elegir la mejor para cada tile;
- **reservar** unos tiles en sitios fijos que no se compartan (el noren, que se anima);
- **quedarse en 512 tiles** aunque el dibujo pida más.

Así que `scripts/preparar_aldea.py` hace la conversión entero y escribe los bytes tal como
los lee la SNES (`res/aldea/cerca_til.bin`, `cerca_pal.bin`, `cerca_col.bin`,
`noren_til.bin`). La lejana sí sigue con gfx4snes (`lejos.png`, regla del `Makefile`). Los
archivos llevan `.bin` porque el `.gitignore` ignora los `.pal` y `.col` de gfx4snes y estos
sí se versionan (el arte de Pixellab de origen **no** se versiona: vive en
`res/descargas/graficos/pixellab/aldea/`).

### Componer en la rejilla de 8

`componer` (`preparar_aldea.py:193`) pega las piezas de Pixellab en un lienzo de 2048×256:
la calle (128 px, repetida **en espejo una vez sí y otra no**, para que los bordes que se
tocan sean iguales: `preparar_calle`, `:184`), las fachadas, los puestos, los aldeanos, y
en otra capa el primer plano (farol y barriles). Todas las x son **múltiplos de 8** (o la
misma x módulo 8 para todas las copias de una pieza). La razón: un tile es un cuadrado fijo
de la rejilla de 8×8 del mapa. Si una fachada se pega en x = 0 y su copia en x = 640, los
dos trozos caen en las mismas posiciones dentro de sus tiles y **dan los mismos tiles**; si
la copia fuera a x = 643, cada tile de la copia sería distinto (el dibujo desplazado 3 px) y
costaría el doble.

### Tiles sin repetidos y el volteo gratis

Después, el lienzo se parte en celdas de 8×8 y para cada una se busca si **ya existe** un
tile igual (`Juego.buscar`, `:314`). La palabra del mapa de la SNES (unidad 06) tiene dos
bits de **volteo**: bit 14 = espejo horizontal, bit 15 = boca abajo. Así que un tile que es
el espejo de otro **no se guarda**: se reutiliza el otro con el bit puesto. `buscar` prueba
las cuatro variantes (tal cual, h, v, h+v) antes de añadir uno nuevo, y por eso las fachadas
volteadas de la calle (`SECUENCIA`, con `True`) casi no cuestan tiles. Es la parte A del
ejercicio.

La demo cuenta los números reales: de **8192** celdas, **4210** son el tile 0 (vacío), el
tile más repetido después del vacío sale **422** veces, y **1398** celdas dibujan un tile
volteado.

### Dos paletas en una capa

Cada celda del mapa dice también **qué paleta** usa (bits 10–12). Una capa de 4bpp no está
atada a 15 colores: cada tile puede tener su paleta. La aldea usa dos (las que se pidieron a
Pixellab): `PALETA_ARQ` (madera, yeso, tejas, tierra) y `PALETA_GENTE` (pieles y ropa de los
aldeanos). `tile_de` (`:281`) convierte cada celda con las dos y se queda con la de **menos
error**. Los aldeanos que tapan una fachada se pintan con la paleta de la gente, que trae
también los marrones de la madera, para que los píxeles de pared que caen en su tile no se
estropeen. Resultado: 8096 celdas con la paleta 4 y 96 con la 5. En la CGRAM van en las
paletas **4 y 5** (colores 64..95): la 0 y la 1 son del texto (BG3 de 2bpp usa 32 colores) y
la 2, de la lejana.

### El límite: 512 tiles y qué hacer al pasarse

La capa cercana tiene en la VRAM de `0x2000` a `0x3FFF` (palabras): 8192 palabras = 16 KB =
**512 tiles** de 32 bytes. La aldea compuesta da **601** tiles distintos. `reducir` (`:333`)
junta, uno a uno, el par de tiles **más parecidos** (menos píxeles distintos, con la misma
paleta) hasta que quedan 512: el que menos se usa pasa a dibujarse con el otro. Hoy se juntan
**89**. Los tiles del noren están fuera de la cuenta (no se tocan). Es una pérdida pequeña y
medida: el script dice cuántos juntó, y la vista `res/aldea/cerca_vista.png` enseña el
resultado tal como lo dibujará la SNES. Si un cambio de composición hace subir mucho ese
número, se nota: es la parte B del ejercicio (un farol más: de 89 a **113**).

### El mapa por columnas

El mapa sale **columna a columna** (`mapa_columnas`, `:423`): las 32 palabras de una columna
son 64 bytes seguidos. Es el formato del anillo de la unidad 07: cuando la cámara entra en una
columna nueva, `fondo_nivel.c` (`apuntar_columna`, `:86`) apunta **un** DMA de 64 bytes en la
lista. La aldea usa el mismo código que el nivel de plataformas; solo cambia el origen
(dirección y banco), que fija la carga: `fondo_nivel_cargar` (nivel 1) o
`fondo_nivel_cargar_aldea` (`:147`), que además copia las **dos** paletas (64 bytes) desde la 4.

### El primer plano que tapa: prioridad del mapa contra prioridad del sprite

En Mode 1, de delante a atrás, la PPU dibuja (con BG3 en prioridad alta, como el juego):

```text
BG3 alta · sprites prio 3 · BG1 alta · BG2 alta · sprites prio 2 · BG1 baja · BG2 baja · sprites prio 1 · ...
```

«Alta» es una celda del mapa con el **bit 13** encendido. El script lo enciende en toda celda
donde el primer plano (farol, barriles) tiene algún píxel. Y los sprites de la fase (la
protagonista, los ninjas, sus sombras, el humo y los pájaros) pasaron de prioridad 3 a **2**
(`PRIORIDAD 2` en `protagonista_sprite.c:48` y `ninja_sprite.c:52`): con 3 irían por delante
de todo menos del texto; con 2 quedan por delante de la calle y las casas (BG1 baja) pero
**por detrás** del farol (BG1 alta).

La prioridad es **por celda**, no por píxel: toda la celda de 8×8 del farol tapa al sprite,
incluidos los píxeles de fachada que caen dentro. Por eso alrededor del poste se ve una
**franja** que corta a Tarou (captura `mesen_farol_tapa_a_tarou.png`). Se disimula con objetos
que llenen sus celdas (el poste ocupa casi toda su columna; los barriles, 32×32 alineados).
La demo lista las columnas con prioridad: 3 columnas por farol (28 celdas) y 4 por barriles.

### Animación de tiles: el noren

El **noren** (la cortina de los puestos) ondea sin tocar el mapa: lo que cambia son **sus
tiles en la VRAM**. El script reserva los 8 tiles de la cortina en los números **1..8**, sin
compartirlos con nada (si un tile del noren se hubiera juntado con otro de una fachada,
animarlo movería también la fachada), y escribe en `noren_til.bin` dos cuadros: el original
y otro con la mitad de abajo movida un píxel (`preparar_puesto`, `:165`). Cada 8 frames,
`pantalla_fase1.c` (`:403`) llama a `fondo_nivel_noren` (`fondo_nivel.c:160`), que apunta en
la lista de DMA 256 bytes desde el cuadro que toca a la dirección de VRAM del tile 1. Como
los seis puestos usan los mismos 8 tiles, **ondean todos a la vez** por el precio de uno.

### No pasarse del VBlank

La lista de DMA se ejecuta en el VBlank, que tiene 37 líneas (unidad 04). Los frames en que
se sube un cuadro de la protagonista y otro de un ninja ya llevan unos 4,5 KB. El noren no es
urgente: si `dma_lista_bytes()` (`dma_lista.c:45`, la suma de lo apuntado) pasa de 4096, ese
frame no lo apunta y lo intenta en el siguiente (`NOREN_HUECO`, `pantalla_fase1.c:95`). El
4096 es una estimación con margen, no una medida.

### La lejana y el cielo

La lejana es una sola imagen de Pixellab de 512 px que **se repite** (el mapa de BG2 es de
64×32: 512 px). Al repetirse, el borde derecho toca al izquierdo y no casaban; el inpainting
de Pixellab tampoco lo arregló, así que `tapar_costura` (`:457`) pinta un monte oscuro
centrado en la costura que la tapa entera desde su cima. Se mueve a la mitad de la cámara
(`bgSetScroll(1, camara >> 1, 0)` en el VBlank). El cielo no es una capa: es el **color de
fondo** (color 0) cambiado en cada línea por HDMA (unidad 12) desde `atardecer.bmp`
(`cielo`, `:505`), con el degradado apretado en la franja que se ve sobre las montañas
(líneas 0..55).

### Sprites del decorado: el humo y los pájaros

El humo y los pájaros **no son fondo**: si fueran tiles animados costarían VRAM de la capa y
se moverían con ella. Son sprites de 16×16 (unidad 21), con su lógica pura en
`juego/ambiente.c` (pruebas en `tests/prueba_ambiente.c`):

- **El humo** sale de los tejados que dice la tabla `aldea_humo_x/_y` de `aldea_datos.h`, que
  **escribe el script** (`fuentes_humo`, `:225`: uno de cada dos tejados, justo encima del
  tejado en la columna `HUMO_X`). Así, si cambia la composición, el humo se recoloca solo.
  Cada tejado a la vista echa dos bocanadas desfasadas que suben 64 px en 128 frames y se
  apartan 8 a la derecha; al llegar arriba renacen. Como los tejados con humo están a 271 y
  369 px, nunca hay más de dos a la vista: **4 entidades** bastan.
- **Los pájaros** vuelan en el plano de las montañas: su x de pantalla es
  `(bandada + separación − cámara/2) & 511`, la misma cuenta que el scroll de la lejana, así
  que parecen tan lejos como las montañas; `& 511` les da la vuelta como a la imagen.

`ambiente_sprite.c` los pone en las entradas 8..14 de la tabla de sprites, una pieza cada
uno, en los tiles 484..491 / 500..507 (los últimos libres del par 15, detrás de la sombra) y
con la **paleta 7**: por la unidad 25, translúcidos sobre las casas y opacos sobre el cielo.
En pausa no se actualizan: se quedan quietos.

## La librería: PVSnesLib

| Función (firma exacta) | Qué hace / registros | Dónde |
|---|---|---|
| `void bgInitTileSet(u8 bgNumber, u8 *tileSource, u8 *tilePalette, u8 paletteEntry, u16 tileSize, u16 paletteSize, u16 colorMode, u16 address);` ([background.h](https://alekmaul.github.io/pvsneslib/background_8h.html)) | copia tiles a la VRAM y la paleta a la CGRAM desde `paletteEntry · 16`; la aldea pasa `paletteSize` = 64 (dos paletas) | `fondo_nivel_cargar_aldea` |
| `void bgSetMapPtr(u8 bgNumber, u16 address, u8 mapSize);` | dónde está el mapa del fondo y su tamaño (`SC_64x32`) | `poner_mapa` (`fondo_nivel.c:130`) |
| `void bgSetScroll(u8 bgNumber, u16 x, u16 y);` | scroll, en el acto: solo en el VBlank | `pantalla_fase1_vblank` |
| `void setModeHdmaColor(u8 channel, u8 *hdmatable);` | el degradado del cielo (`hdmaGradatardecerList`) | `pantalla_fase1_iniciar` |
| `void dmaCopyVram(u8 *source, u16 address, u16 size);` / `dmaCopyCGram` ([dma.h](https://alekmaul.github.io/pvsneslib/dma_8h.html)) | los tiles y la paleta del humo y los pájaros, al cargar | `ambiente_sprite_cargar` |

El mapa por columnas, el noren y las columnas del anillo **no** usan la librería: van por la
lista de DMA del juego (`dma_lista_anadir`, `dma_ejecutar.asm`, unidad 07), porque son copias
pequeñas que hay que hacer **todas juntas** en el VBlank.

## Dónde está en el juego

| Qué buscar | Dónde |
|---|---|
| Qué pieza de Pixellab se usa y dónde va cada una | `FUENTES`, `SECUENCIA`, `ALDEANOS`, `FAROLES`, `BARRILES` en `scripts/preparar_aldea.py` (arriba) |
| Componer, calle en espejo, puesto y noren | `componer` `:193`, `preparar_calle` `:184`, `preparar_puesto` `:165` |
| Tiles sin repetidos, reserva, reducción | `Juego` `:298` (`buscar` `:314`, `reservar` `:307`), `reducir` `:333` |
| Palabra del mapa, conversión entera, por columnas | `entrada` `:378`, `convertir` `:383`, `mapa_columnas` `:423` |
| Costura de la lejana, cielo | `tapar_costura` `:457`, `lejana` `:478`, `cielo` `:505` |
| Humo: de qué tejados; tiles de humo y pájaros | `fuentes_humo` `:225`, `cuadros_16` `:240`, `tiles_ambiente` `:255` |
| Carga de la aldea y animación del noren | `src/juego/fondo_nivel.c:147`, `:160` |
| El noren cada 8 frames y el hueco del VBlank | `src/pantallas/pantalla_fase1.c:403` (`NOREN_HUECO` `:95`), `dma_lista_bytes` |
| Humo y pájaros | `src/juego/ambiente.c:70`, `src/juego/ambiente_sprite.c:35` y `:58` |
| Lo generado | `res/aldea/` (`aldea_datos.h` dice cuántos tiles, dónde el noren y los tejados del humo) |
| Pruebas | `scripts/pruebas/prueba_preparar_aldea.py` (17), `tests/prueba_ambiente.c` |

## Trampas

| Trampa | Qué pasa | Cómo se evita |
|---|---|---|
| Pegar una pieza en una x que no es múltiplo de 8 | cada copia da tiles nuevos: se dispara la cuenta | rejilla de 8; el script lo dice con «juntados» |
| Compartir un tile animado | al animarlo cambia también otra parte del fondo | `reservar` antes de convertir lo demás |
| Poner prioridad en celdas a medio llenar | una franja de fachada tapa al personaje | objetos que llenen sus celdas, alineados |
| Dejar los sprites en prioridad 3 | el primer plano no tapa a nadie | prioridad 2 en la fase |
| Editar `res/aldea/*.bin` a mano | el siguiente `preparar_aldea.py` lo pisa | cambiar el script y regenerar |
| Regenerar sin el arte de Pixellab | el script se para: «faltan piezas» | el arte está solo en `res/descargas/` del autor: copiarlo al clon |
| Cambiar `aldea_datos.h` y no hacer `make clean` | `snes_rules` no sigue las cabeceras | `make clean` (o las dependencias del `Makefile` para `fondo_nivel.obj`, `ambiente.obj`, `pantalla_fase1.obj`) |
| Más adornos que entidades | `SPRITES_MAX` es 16 y la fase usa 15 | contar antes de añadir |

## La demo

`demo_27_aldea.py` lee `res/aldea/` **sin usar el script**, interpretando los bytes como la
PPU. Desde esta carpeta:

```text
python demo_27_aldea.py
```

Salida real en `capturas/salida_demo_27_aldea.txt`: A, el juego de tiles (512, dos paletas
desde la 4); B, el mapa (celdas vacías, repetidas, volteadas, por paleta); C, las columnas
con prioridad (los cuatro faroles y los tres barriles); D, el noren (tiles 1..8, 80 bytes de
256 cambian, 48 celdas: 6 puestos); E, lo que cuesta copiar; F, los tejados con humo y sus
separaciones.

Antes de ejecutarla, predice: ¿cuántas celdas del mapa usan los tiles del noren? (8 por
puesto × 6 puestos = 48.)

## Ejercicio y criterio de aprobación

**Ejercicio (`ejercicio_27.md`), en dos partes.** **A (PC, Python con TDD):** escribe primero
las pruebas de un juego de tiles con volteos y luego complétalo (`ejercicio_27_tiles.py`).
**B (juego real, en un clon):** añade un farol al primer plano, regenera la aldea, mira cuánto
cuesta en tiles y compruébalo en Mesen.

**Aprobación:**

1. la demo da la salida de `capturas/salida_demo_27_aldea.txt`;
2. parte A: con tus pruebas y el esqueleto, **7 de 9 fallan**
   (`capturas/pruebas_27_rojo.txt`); completado, **9 OK** (`capturas/pruebas_27_verde.txt`);
3. parte B: regenerar sin cambios no cambia ningún archivo (`git status` limpio); con el farol
   en x = 300, el script dice **juntados 113** (antes 89: `capturas/preparar_antes.txt`,
   `preparar_con_farol.txt`); la ROM `-DGUION_FASE1` compila sin avisos
   (`capturas/compilacion.txt`) y en Mesen el farol nuevo tapa a Tarou
   (`capturas/mesen_farol_nuevo.png`, `mesen_farol_tapa_a_tarou.png`).

## Por venir (no enseñar como hecho)

| Lo que dicen `docs/arte-beat-em-up.md` §5 y `docs/pixellab-fase1.md` §6 | Estado hoy |
|---|---|
| Bosque de bambú, monasterio, dojo (otras zonas con su tileset) | no existe: toda la fase es la aldea |
| Cambiar de tileset a mitad de fase (pantalla apagada) | no existe |
| Lejana por franjas (montañas a 1/4, tejados a 1/2) con HDMA sobre `BG2HOFS` | no existe: toda a 1/2 (decisión del 2026-11-05) |
| Objetos rompibles (barriles, faroles) | el farol y los barriles son fondo: no se rompen |
| Más adornos (hojas, luciérnagas) | no existe |
