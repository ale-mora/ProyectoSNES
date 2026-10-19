#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    preparar_suelo.py — Construye la capa cercana del nivel 1 (el suelo donde
    camina el protagonista) a partir de un tileset «Sidescroller» de Pixellab.

    PROPÓSITO
      Pixellab (Create Tileset → Sidescroller) exporta una hoja de tiles de
      16x16 con muchas piezas de plataforma. Para un suelo plano bastan dos
      tipos: el BORDE (filas de arriba vacías, hierba, tierra debajo) y la
      TIERRA (un tile opaco). Este script los encuentra solo, pinta con ellos
      una franja de suelo de 512 px de ancho cuya hierba empieza exactamente en
      la fila SUELO_Y del juego, rellena de tierra hasta abajo y varía la
      tierra con volteos (gratis en el mapa de la SNES) para que no se vean
      franjas. Opcionalmente pone detrás los arbustos del título subidos, como
      un seto. El resultado es una imagen indexada de 15 colores + transparente
      que gfx4snes convierte en tiles y mapa.

    SE CONECTA CON
      Lee:      res/descargas/graficos/pixellab/suelo/*.png (la exportación) y,
                con --seto, res/titulo/fondo_cerca.png (los arbustos).
      Escribe:  res/nivel1/fondo_cerca.png (512x256, indexada).
      Usa:      scripts/preparar_personaje.py (indexar: color más cercano).
      Lo usan:  el Makefile (gfx4snes -y ... res/nivel1/fondo_cerca.pic),
                data.asm (.rodata_nivel1_cerca), src/pantallas/pantalla_nivel1.c
                (capa 0, scroll 1:1 con la cámara) y
                scripts/pruebas/prueba_preparar_suelo.py.

    HARDWARE QUE TOCA
      ninguno; produce datos. Reglas de la PPU que respeta: índice 0 =
      transparente (se ven las montañas y el cielo HDMA detrás), 16 colores por
      paleta en Mode 1, alto múltiplo de 256 para el mapa en páginas (-y).

    VARIABLES (constantes del módulo)
      T          int   16 px    lado de los tiles de la hoja de Pixellab
      ANCHO      int   512 px   ancho del nivel (ANCHO_NIVEL)
      ALTO       int   256 px   alto de la imagen (224 visibles + relleno para -y)
      VISIBLE    int   224 px   filas que se ven en pantalla
      SEMILLA    int   2026     semilla fija de la variación (salida reproducible)

    Uso (desde la raíz del repo):
      python scripts/preparar_suelo.py --seto res/titulo/fondo_cerca.png
      python scripts/preparar_suelo.py RUTA_HOJA.png --suelo 176 --salida res/nivel1/fondo_cerca.png
    → glosario: Tile, Tilemap, Paleta, Parallax, Suelo
---------------------------------------------------------------------------------
"""

import argparse
import random
import sys
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
import preparar_personaje as pp  # noqa: E402  (indexar: color más cercano, índice 0 transparente)

T = 16
ANCHO = 512
ALTO = 256
VISIBLE = 224
SEMILLA = 2026
CARPETA = Path("res/descargas/graficos/pixellab/suelo")


# --- 1. Encontrar los tiles de BORDE y de TIERRA en la hoja -------------------------

def opacos(tile: Image.Image) -> list:
    """Cuántos píxeles opacos tiene cada fila del tile."""
    a = tile.getchannel("A").load()
    return [sum(1 for x in range(T) if a[x, y] > 0) for y in range(T)]


def es_verde(c) -> bool:
    r, g, b, a = c
    return a > 0 and g > r + 30 and g > b + 30


def clasificar(hoja: Image.Image):
    """Devuelve (bordes, tierras): listas de tiles. BORDE = primera fila vacía y las
    6 de abajo llenas (hierba sobre tierra). TIERRA = 256 píxeles opacos y nada
    verde. De los bordes se toma la tira horizontal más larga, en orden, para que
    las uniones entre tiles sean las que dibujó Pixellab."""
    cols, filas = hoja.width // T, hoja.height // T
    tiles = {(c, f): hoja.crop((c * T, f * T, c * T + T, f * T + T)) for f in range(filas) for c in range(cols)}
    es_borde, tierras = {}, []
    for (c, f), t in tiles.items():
        o = opacos(t)
        verde = any(es_verde(p) for p in (t.get_flattened_data() if hasattr(t, "get_flattened_data") else t.getdata()))
        es_borde[(c, f)] = o[0] == 0 and all(n == T for n in o[T - 6:])
        if all(n == T for n in o) and not verde:
            tierras.append(t)
    mejor = []
    for f in range(filas):
        tira = []
        for c in range(cols + 1):
            if c < cols and es_borde[(c, f)]:
                tira.append(tiles[(c, f)])
            else:
                if len(tira) > len(mejor):
                    mejor = tira
                tira = []
    return mejor, tierras


# --- 2. Paleta: los colores del seto tal cual + los del suelo reducidos ------------

def reducir(colores: list, n: int) -> list:
    """Como mucho n colores representativos (median cut, sin tramado)."""
    unicos = sorted(set(colores))
    if len(unicos) <= n:
        return unicos
    fila = Image.new("RGB", (len(colores), 1))
    fila.putdata(colores)
    q = fila.quantize(colors=n, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    pal = q.getpalette()[: n * 3]
    return sorted(set(tuple(pal[i * 3: i * 3 + 3]) for i in range(n)))


# --- 3. Todo junto -------------------------------------------------------------------

def preparar(hoja_ruta: Path, salida: Path, suelo_y: int = 176, seto: Path | None = None) -> None:
    """Escribe la capa cercana: (seto) + borde con la hierba en la fila suelo_y +
    tierra variada hasta abajo. No escribe nada si la hoja no tiene bordes o tierra."""
    with Image.open(hoja_ruta) as im:
        hoja = im.convert("RGBA")
    bordes, tierras = clasificar(hoja)
    if not bordes or not tierras:
        sys.exit(f"ERROR: {hoja_ruta} no tiene tiles de borde (hierba sobre tierra) o de tierra "
                 f"opaca de {T}x{T}. ¿Es una exportación Sidescroller? No se escribió nada.")
    r0 = min(next(y for y, n in enumerate(opacos(t)) if n) for t in bordes)   # primera fila con hierba

    capa = Image.new("RGBA", (ANCHO, ALTO), (0, 0, 0, 0))
    colores_seto = []
    if seto is not None:
        # Los arbustos se suben hasta que su base quede 16 px por debajo de la hierba:
        # la tierra, dibujada después, la tapa y solo asoman las copas, como un seto.
        with Image.open(seto) as s:
            arbustos = s.convert("RGBA")
            if s.mode == "P":                         # el índice 0 es el transparente
                # Máscara desde los ÍNDICES (bytes de la imagen), no desde los colores:
                # convertir P -> L usaría la paleta y el índice 0 podría no ser negro.
                indices = Image.frombytes("L", s.size, s.tobytes())
                arbustos.putalpha(Image.eval(indices, lambda i: 255 if i else 0))
        caja = arbustos.getchannel("A").getbbox()
        if caja:
            subir = caja[3] - (suelo_y + T)
            capa.alpha_composite(arbustos.crop((0, subir, arbustos.width, arbustos.height)), (0, 0))
            colores_seto = sorted({p[:3] for p in (arbustos.get_flattened_data() if hasattr(arbustos, "get_flattened_data") else arbustos.getdata()) if p[3] > 0})

    y_borde = suelo_y - r0                             # la fila r0 del tile cae en suelo_y
    for k in range(ANCHO // T):
        capa.alpha_composite(bordes[k % len(bordes)], (k * T, y_borde))
    azar = random.Random(SEMILLA)
    volteos = [lambda t: t, lambda t: t.transpose(Image.FLIP_LEFT_RIGHT),
               lambda t: t.transpose(Image.FLIP_TOP_BOTTOM), lambda t: t.transpose(Image.ROTATE_180)]
    y = y_borde + T
    while y < ALTO:
        for k in range(ANCHO // T):
            t = azar.choice(volteos)(azar.choice(tierras))
            capa.alpha_composite(t, (k * T, y))
        y += T

    # Paleta: el seto conserva sus colores; el suelo se reduce a lo que quede de 15.
    suelo = capa.crop((0, suelo_y, ANCHO, VISIBLE))
    datos = suelo.get_flattened_data() if hasattr(suelo, "get_flattened_data") else suelo.getdata()
    colores_suelo = reducir([p[:3] for p in datos if p[3] > 0], pp.COLORES - len(colores_seto))
    colores = colores_seto + [c for c in colores_suelo if c not in colores_seto]
    salida.parent.mkdir(parents=True, exist_ok=True)
    pp.indexar(capa, colores).save(salida, format="PNG")
    print(f"  {salida}  {ANCHO}x{ALTO}; borde de {len(bordes)} tiles, {len(tierras)} de tierra; "
          f"hierba en la fila {suelo_y}; {len(colores)} colores + transparente"
          + (f" (seto: {len(colores_seto)})" if colores_seto else ""))


def main() -> int:
    ap = argparse.ArgumentParser(description="Capa cercana del nivel 1: suelo de Pixellab (+ seto).")
    ap.add_argument("hoja", type=Path, nargs="?", default=None,
                    help=f"hoja de tiles de Pixellab (por defecto, el primer PNG de {CARPETA})")
    ap.add_argument("--salida", type=Path, default=Path("res/nivel1/fondo_cerca.png"))
    ap.add_argument("--suelo", type=int, default=176, help="fila donde empieza la hierba (SUELO_Y del juego)")
    ap.add_argument("--seto", type=Path, default=None, help="arbustos a poner detrás (p. ej. res/titulo/fondo_cerca.png)")
    args = ap.parse_args()
    hoja = args.hoja
    if hoja is None:
        pngs = sorted(CARPETA.glob("*.png"))
        if not pngs:
            sys.exit(f"ERROR: no hay ningún PNG en {CARPETA}")
        hoja = pngs[0]
    print(f"[suelo] hoja: {hoja}")
    preparar(hoja, args.salida, args.suelo, args.seto)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
