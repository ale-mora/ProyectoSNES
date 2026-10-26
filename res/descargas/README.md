# res/descargas — buzón de assets sin procesar

Deposita aquí lo que descargues **tal cual viene**. Nada de esta carpeta se
compila ni se versiona en git (salvo este README): es material de terceros
para un proyecto personal sin distribución.

```
res/descargas/
  graficos/   PNG, BMP, GIF, JPG... logos, fondos, spritesheets
  audio/      .it, .xm, .mod, .s3m, .wav ... música y efectos
```

Después, cada archivo se **prepara** (indexado, tamaño, colores) y se copia a
la carpeta de la pantalla que lo usa (`res/nintendo/`, `res/intro/`, etc.).
Eso lo hace Claude con `scripts/preparar_imagen.py`; solo hace falta decir qué
archivo es para qué pantalla.

## Qué necesita cada pantalla

| Pantalla | Archivo(s) a conseguir | Tamaño ideal | Notas |
| --- | --- | --- | --- |
| Nintendo | logo "Nintendo" rojo sobre negro | 256×224 | puede ser una captura de la pantalla de arranque de un juego |
| Publisher | ya cubierto por el ejemplo Capcom del SDK | — | opcional: otro logo de época |
| Developers | logo de estudio (estilo Konami/época) | 256×224 | irá a Mode 7 (256 colores): puede ser detallado |
| Intro | 3 fondos de escena (pueblo, camino, colinas...) | 256×224 | máx. 16 colores tras preparar; estilo _Secret of Mana_ |
| Intro / Título | spritesheet de un cachorro (quieto, mueve la cola) | cuadros de 16×16 o 32×32 en fila | 16 colores, fondo transparente o color plano |
| Título | 2 fondos: primer plano y lejano | 256×224 o 512×224 | el lejano puede ser cielo/montañas; el degradado lo hace la SNES |

## Formatos de audio que sirven y que no

| Sirve | No sirve |
| --- | --- |
| **`.it`** (Impulse Tracker): música y jingles | `.spc` (rips de SNES): no se pueden reproducir con PVSnesLib |
| `.xm`, `.mod`, `.s3m`: se convierten a `.it` con OpenMPT (gratis) | `.mp3`, `.ogg`, `.flac`: no |
| `.wav` **cortos** mono 16 bits: efectos de sonido | `.wav` largos como música: no (la RAM de audio es de 64 KB) |

Límites de la música `.it`: 8 canales como máximo, muestras de hasta 58 KB tras
comprimir, y el total música + efectos por debajo de 60 KB. Dónde buscar:
`modarchive.org` (filtrar por formato IT), y los `.it` de ejemplo del SDK en
`tools/pvsneslib/snes-examples/audio/` y `.../thirdparty/logo/`.

## Formatos de imagen

`gfx4snes` acepta **PNG o BMP indexados de 256 colores**. Cualquier PNG/JPG
normal se convierte con:

```
python scripts/preparar_imagen.py res/descargas/graficos/foto.png res/intro/escena_1.png --colores 16 --ancho 256 --alto 224
```
