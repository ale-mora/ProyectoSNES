#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    preparar_zona.py — Las zonas 2, 3 y 4 de la fase 1 (bosque de bambú, monasterio de
    Rex y dojo) para la SNES, con el mismo formato que la aldea.

    PROPÓSITO
      Como scripts/preparar_aldea.py pero con una configuración por zona (ZONAS): cada
      zona mide 1024 px y se compone en la rejilla de 8 con piezas de Pixellab
      (res/descargas/graficos/pixellab/<zona>/, ignorado por git): el suelo (en espejo
      alterno, sin costura), dos módulos de fondo que se alternan (y sus espejos), un
      elemento de primer plano (con el bit de prioridad: tapa a los personajes) y, en el
      monasterio, el torii. La paleta de la cercana y la de la lejana salen de
      <zona>/paletas.txt (las que se pidieron a Pixellab). Escribe en res/zonas/<zona>/:
        cerca_til.bin, cerca_pal.bin (la paleta, dos veces: el cargador copia dos),
        cerca_col.bin (128 columnas x 32 filas, columna a columna), lejos.png (512x256
        indexado, la costura tapada), cielo<zona>.bmp (un color por línea) y
        cerca_vista.png; y res/zonas/zonas_datos.h con los tiles de cada zona.

    SE CONECTA CON
      Usa:      scripts/preparar_aldea.py (pegar, opacar, Juego, tile_de, reducir, entrada,
                indices_tile, hex_rgb, paleta_bytes), scripts/piezas16.py (tile_4bpp).
      Lo usan:  el Makefile (lejos.png, cielo*.bmp), data.asm (los .bin), juego/fondo_nivel.c.

    HARDWARE QUE TOCA
      ninguno; produce datos con el formato de la VRAM (ver preparar_aldea.py).

    VARIABLES: ZONAS (config por zona). Uso: python scripts/preparar_zona.py [zona ...]
    → glosario: Tile, Tilemap, Paleta, Prioridad
---------------------------------------------------------------------------------
"""
import json
import struct
import sys
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
from piezas16 import tile_4bpp  # noqa: E402
import preparar_aldea as al  # noqa: E402

RAIZ = Path(__file__).resolve().parents[1]
ORIGEN = RAIZ / "res/descargas/graficos/pixellab"
DESTINO = RAIZ / "res/zonas"
ANCHO = 1024
ALTO = 256
COLUMNAS = ANCHO // 8
FILAS = ALTO // 8
SUELO_Y = 176
PALETA_SNES = 4

ZONAS = {
    "bosque": {
        "suelo": "suelo/04.png", "modulos": ["modulo_a/01.png", "modulo_b/02.png"],
        "frente": "frente/02.png", "lejana": "lejana/01.png", "hueco": 24,
        "frentes": [180, 452, 724, 964], "torii": None,
        "cielo": [(0, "#20343a"), (40, "#3c6a62"), (80, "#7aa88e"), (223, "#c8e0c0")],
    },
    "monasterio": {
        "suelo": "suelo/01.png", "modulos": ["modulo_a/01.png", "modulo_b/03.png"],
        "frente": "frente/03.png", "lejana": "lejana/00.png", "hueco": 16,
        "frentes": [244, 620, 900], "torii": ("torii/00.png", 400),
        "cielo": [(0, "#060818"), (40, "#101838"), (90, "#202c58"), (223, "#303c68")],
    },
    "dojo": {
        "suelo": "suelo/07.png", "modulos": ["modulo_a/01.png", "modulo_b/01.png"],
        "frente": "frente/01.png", "lejana": "lejana/00.png", "hueco": 0,
        "frentes": [300, 700], "torii": None,
        "cielo": [(0, "#281c4c"), (30, "#6a3070"), (60, "#c05868"), (223, "#f0a060")],
    },
}


def paletas(zona):
    """CERCANA y LEJANA de <zona>/paletas.txt (dos listas de 15 colores)."""
    cerca = lejos = None
    for linea in (ORIGEN / zona / "paletas.txt").read_text(encoding="utf-8").splitlines():
        if linea.startswith("CERCANA:"):
            cerca = json.loads(linea.split(":", 1)[1])
        elif linea.startswith("LEJANA:"):
            lejos = json.loads(linea.split(":", 1)[1])
    return cerca, lejos


def abrir(zona, archivo):
    return al.opacar(Image.open(ORIGEN / zona / archivo).convert("RGBA"))


def pie(img):
    """Fila del borde de abajo de lo dibujado (para posarlo en el suelo)."""
    return img.getbbox()[3]


def componer(zona, cfg):
    fondo = Image.new("RGBA", (ANCHO, ALTO), (0, 0, 0, 0))
    frente = Image.new("RGBA", (ANCHO, ALTO), (0, 0, 0, 0))
    suelo = abrir(zona, cfg["suelo"])
    a = np.array(suelo)
    a[:, -1] = a[:, -2]                              # Sin la columna de marco (si la hay).
    suelo = Image.fromarray(a, "RGBA")
    for k, x in enumerate(range(0, ANCHO, suelo.width)):
        al.pegar(fondo, suelo, x, SUELO_Y, voltear=bool(k & 1))
    modulos = [abrir(zona, m) for m in cfg["modulos"]]
    x, i = 0, 0
    torii_x = cfg["torii"][1] if cfg["torii"] else None
    while x < ANCHO:
        if torii_x is not None and x <= torii_x < x + 128:
            t = abrir(zona, cfg["torii"][0])
            al.pegar(fondo, t, x, SUELO_Y - pie(t))
            x += t.width + cfg["hueco"]
            torii_x = None
            continue
        m = modulos[i % len(modulos)]
        al.pegar(fondo, m, x, SUELO_Y - pie(m), voltear=bool((i >> 1) & 1))   # A, B, A', B'...
        x += m.width + cfg["hueco"]
        i += 1
    f = abrir(zona, cfg["frente"])
    for fx in cfg["frentes"]:
        al.pegar(frente, f, fx, al.FRENTE_PIE - pie(f))   # Detrás del carril (ver preparar_aldea).
    return fondo, frente


def convertir(fondo, frente, paleta):
    img = Image.alpha_composite(fondo, frente)
    a = np.array(img)
    prio = np.array(frente)[..., 3] > 0
    juego = al.Juego()
    mapa = [[None] * COLUMNAS for _ in range(FILAS)]
    for cy in range(FILAS):
        for cx in range(COLUMNAS):
            p = int(prio[cy * 8:cy * 8 + 8, cx * 8:cx * 8 + 8].any()) & al.FRENTE_PRIORIDAD
            ind, pal = al.tile_de(a, cx, cy, [paleta])
            t, h, v = juego.buscar(ind, pal)
            mapa[cy][cx] = (t, pal, p, h, v)
    juntados = al.reducir(juego, mapa, al.LIMITE_TILES, 1)
    return juego, mapa, juntados


def columnas(mapa):
    vals = []
    for cx in range(COLUMNAS):
        for cy in range(FILAS):
            t, pal, p, h, v = mapa[cy][cx]
            vals.append(al.entrada(t, PALETA_SNES, p, h, v))
    return struct.pack(f"<{len(vals)}H", *vals)


def tapar_costura(img, oscuro, borde, ancho_monte=80, cima=40):
    a = np.array(img)
    w, h = img.width, img.height
    rng = np.random.default_rng(11)
    ruido = np.cumsum(rng.integers(-1, 2, size=2 * ancho_monte + 1))
    for k, dx in enumerate(range(-ancho_monte, ancho_monte + 1)):
        t = abs(dx) / ancho_monte
        y_top = max(cima - 4, min(h, int(cima + (h - cima) * (t ** 1.4) + ruido[k] * 0.6)))
        x = dx % w
        a[y_top:, x] = list(al.hex_rgb(oscuro)) + [255]
        if y_top < h and dx < 0:
            a[y_top, x] = list(al.hex_rgb(borde)) + [255]
    return Image.fromarray(a, "RGBA")


def lejana(zona, cfg, paleta):
    p = abrir(zona, cfg["lejana"])
    p = tapar_costura(p, paleta[0], paleta[2])
    lienzo = Image.new("RGBA", (512, 256), (0, 0, 0, 0))
    al.pegar(lienzo, p, 0, al.LEJOS_Y)
    a = np.array(lienzo)
    a[p.height + al.LEJOS_Y:, :] = list(al.hex_rgb(paleta[0])) + [255]
    ind = np.zeros(a.shape[:2], np.uint8)
    for y in range(a.shape[0]):
        ind[y], _ = al.indices_tile(a[y], paleta)
    ind = limitar_tiles(ind, LIMITE_LEJOS)
    out = Image.fromarray(ind, "P")
    plano = [255, 0, 255]
    for c in paleta:
        plano += list(al.hex_rgb(c))
    out.putpalette(plano + [0] * (768 - len(plano)))
    return out


LIMITE_LEJOS = 496            # VRAM_FONDO_B: 512 tiles; unos pocos de margen.


def limitar_tiles(ind, limite):
    """Junta los tiles menos usados de la lejana con el más parecido hasta que queden
    `limite` distintos (contando volteos, como gfx4snes): la VRAM no tiene más sitio."""
    h, w = ind.shape
    celdas = {}
    for y in range(0, h, 8):
        for x in range(0, w, 8):
            t = ind[y:y + 8, x:x + 8]
            k = min(t.tobytes(), t[:, ::-1].tobytes(), t[::-1].tobytes(), t[::-1, ::-1].tobytes())
            celdas.setdefault(k, []).append((y, x))
    while len(celdas) > limite:
        raro = min(celdas, key=lambda k: len(celdas[k]))
        a = np.frombuffer(raro, np.uint8)
        otro = min((k for k in celdas if k != raro),
                   key=lambda k: int((np.frombuffer(k, np.uint8) != a).sum()))
        nuevo = np.frombuffer(otro, np.uint8).reshape(8, 8)
        for y, x in celdas.pop(raro):
            ind[y:y + 8, x:x + 8] = nuevo
            celdas[otro].append((y, x))
    return ind


def cielo(paradas):
    guardado = al.CIELO
    al.CIELO = paradas
    try:
        return al.cielo()
    finally:
        al.CIELO = guardado


def vista(juego, mapa, paleta):
    pal = [(0, 0, 0)] + [al.hex_rgb(c) for c in paleta]
    out = np.zeros((ALTO, ANCHO, 4), np.uint8)
    for cy in range(FILAS):
        for cx in range(COLUMNAS):
            t, _, _, h, v = mapa[cy][cx]
            m = np.array(juego.tiles[t]).reshape(8, 8)
            if h:
                m = m[:, ::-1]
            if v:
                m = m[::-1, :]
            for y in range(8):
                for x in range(8):
                    if m[y, x]:
                        out[cy * 8 + y, cx * 8 + x] = list(pal[m[y, x]]) + [255]
    return Image.fromarray(out, "RGBA")


def preparar(zona):
    cfg = ZONAS[zona]
    cerca, lejos = paletas(zona)
    fondo, frente = componer(zona, cfg)
    juego, mapa, juntados = convertir(fondo, frente, cerca)
    d = DESTINO / zona
    d.mkdir(parents=True, exist_ok=True)
    (d / "cerca_til.bin").write_bytes(b"".join(tile_4bpp(list(t)) for t in juego.tiles))
    (d / "cerca_pal.bin").write_bytes(al.paleta_bytes(cerca) * 2)
    (d / "cerca_col.bin").write_bytes(columnas(mapa))
    lejana(zona, cfg, lejos).save(d / "lejos.png")
    cielo(cfg["cielo"]).save(d / f"cielo{zona}.bmp")
    vista(juego, mapa, cerca).save(d / "cerca_vista.png")
    print(f"{zona}: {len(juego.tiles)} tiles (juntados {juntados})")
    return len(juego.tiles)


def main(argv):
    zonas = argv or list(ZONAS)
    tiles = {z: preparar(z) for z in zonas}
    lineas = ["/* zonas_datos.h — GENERADO por scripts/preparar_zona.py: no editar a mano.",
              "   Las zonas 2..4 de la fase 1 (res/zonas/<zona>/): 1024 px, 128 columnas. */",
              "#ifndef ZONAS_DATOS_H", "#define ZONAS_DATOS_H", f"#define ZONA_COLUMNAS {COLUMNAS}"]
    for z, n in tiles.items():
        lineas.append(f"#define ZONA_{z.upper()}_TILES {n}")
    lineas += ["#endif", ""]
    (DESTINO / "zonas_datos.h").write_text("\n".join(lineas), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
