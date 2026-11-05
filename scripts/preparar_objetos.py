#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    preparar_objetos.py — Los objetos de la fase (rompibles, comida, armas, astillas y
    chispa) en piezas de 16x16 de 4bpp, con la paleta de sprites 7.

    PROPÓSITO
      El arte llega de Pixellab (res/descargas/graficos/pixellab/objetos/, ignorado por
      git) ya dibujado con PALETA_OBJETOS (la misma del humo y los pájaros: paleta de
      sprites 7). Este script elige el candidato de cada pieza (FUENTES), lo convierte a
      índices de esa paleta y escribe:
        res/objetos/objetos_til.bin  las piezas en orden, 128 bytes cada una: la mitad de
                                     arriba (tiles TL, TR: 64 B) y la de abajo (BL, BR);
                                     un objeto de 32x32 son 4 piezas (TL, TR, BL, BR)
        res/objetos/objetos_datos.h  dónde empieza cada pieza (OBJ_PIEZA_*) y la caja de
                                     lo dibujado de cada objeto (OBJ_CAJA_*: x0, y0, x1, y1;
                                     objetos_sprite.c pone ahí la ventana que lo deja opaco)
        res/objetos/objetos_vista.png  todas, a 4x, para mirarlas

    SE CONECTA CON
      Lee:      res/descargas/graficos/pixellab/objetos/<pieza>/NN.png.
      Usa:      scripts/preparar_aldea.py (PALETA_OBJETOS, indices_tile, opacar),
                scripts/piezas16.py (tile_4bpp).
      Lo usan:  data.asm (objetos_til), src/objetos/objetos_sprite.c (objetos_datos.h);
                scripts/pruebas/prueba_preparar_objetos.py.

    HARDWARE QUE TOCA
      ninguno; produce tiles 4bpp de 32 bytes.

    VARIABLES: FUENTES (pieza → archivo, tamaño). Uso: python scripts/preparar_objetos.py
    → glosario: Tile, Pieza de 16×16, Paleta
---------------------------------------------------------------------------------
"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
from piezas16 import tile_4bpp  # noqa: E402
from preparar_aldea import PALETA_OBJETOS, indices_tile, opacar  # noqa: E402

RAIZ = Path(__file__).resolve().parents[1]
ORIGEN = RAIZ / "res/descargas/graficos/pixellab/objetos"
DESTINO = RAIZ / "res/objetos"

# (nombre, archivo dentro de objetos/, recorte (x, y, ancho, alto) o None = la imagen entera)
FUENTES = [
    ("barril", "barril/00.png", None),            # 32x32: 4 piezas
    ("caja", "caja/11.png", None),
    ("jarra", "jarra/40.png", None),
    ("astillas", "astillas/08.png", None),        # 16x16: 1 pieza
    ("onigiri", "onigiri/01.png", None),
    ("pescado", "pescado/06.png", None),
    ("vida", "vida/24.png", None),
    ("shuriken", "shuriken/sheet_24_25.png", (0, 0, 16, 16)),   # el cuadro de 0 grados (gira con los volteos)
    ("kunai", "kunai/compuestos/20.png", None),
    ("chispa_a", "chispa/09.png", (0, 0, 16, 16)),
    ("chispa_b", "chispa/09.png", (16, 0, 16, 16)),
]


def piezas_de(img):
    """Imagen RGBA de 16x16 o 32x32 → lista de piezas (matrices 16x16 de índices)."""
    a = np.array(opacar(img))
    h, w = a.shape[:2]
    piezas = []
    for py in range(0, h, 16):
        for px in range(0, w, 16):
            pix = a[py:py + 16, px:px + 16].reshape(256, 4)
            ind, _ = indices_tile(pix, PALETA_OBJETOS)
            piezas.append(ind.reshape(16, 16))
    return piezas


# Los que se dibujan volteados (el shuriken gira con los volteos; el kunai y las astillas
# miran a los dos lados): su caja es la unión con sus espejos.
ESPEJO_H = {"shuriken", "kunai", "astillas"}
ESPEJO_V = {"shuriken"}


def caja_de(img, nombre):
    """(x0, y0, x1, y1) inclusivos de los píxeles opacos de la imagen (con sus espejos)."""
    a = np.array(opacar(img))[..., 3] > 0
    if nombre in ESPEJO_H:
        a = a | a[:, ::-1]
    if nombre in ESPEJO_V:
        a = a | a[::-1, :]
    ys, xs = np.nonzero(a)
    if len(xs) == 0:
        return (0, 0, 0, 0)
    return (int(xs.min()), int(ys.min()), int(xs.max()), int(ys.max()))


def bytes_pieza(m):
    """Una pieza 16x16 → 128 bytes: TL, TR (arriba) y BL, BR (abajo)."""
    return (tile_4bpp(list(m[:8, :8].reshape(64))) + tile_4bpp(list(m[:8, 8:].reshape(64))) +
            tile_4bpp(list(m[8:, :8].reshape(64))) + tile_4bpp(list(m[8:, 8:].reshape(64))))


def main():
    faltan = [f for _, f, _ in FUENTES if not (ORIGEN / f).exists()]
    if faltan:
        sys.exit("ERROR: faltan piezas en " + str(ORIGEN) + ": " + ", ".join(faltan))
    todas, indice, cajas = [], [], []
    for nombre, archivo, recorte in FUENTES:
        img = Image.open(ORIGEN / archivo).convert("RGBA")
        if recorte:
            x, y, w, h = recorte
            img = img.crop((x, y, x + w, y + h))
        indice.append((nombre, len(todas)))
        cajas.append((nombre, caja_de(img, nombre)))
        todas += piezas_de(img)
    DESTINO.mkdir(parents=True, exist_ok=True)
    (DESTINO / "objetos_til.bin").write_bytes(b"".join(bytes_pieza(m) for m in todas))
    lineas = ["/* objetos_datos.h — GENERADO por scripts/preparar_objetos.py: no editar a mano.",
              "   La primera pieza de 16x16 de cada objeto en res/objetos/objetos_til.bin (128 bytes",
              "   cada una; los de 32x32 son 4 seguidas: TL, TR, BL, BR). */",
              "#ifndef OBJETOS_DATOS_H", "#define OBJETOS_DATOS_H"]
    for nombre, k in indice:
        lineas.append(f"#define OBJ_PIEZA_{nombre.upper()} {k}")
    for nombre, (x0, y0, x1, y1) in cajas:
        lineas.append(f"#define OBJ_CAJA_{nombre.upper()} {x0}, {y0}, {x1}, {y1}")
    lineas += [f"#define OBJ_PIEZAS {len(todas)}", "#endif", ""]
    (DESTINO / "objetos_datos.h").write_text("\n".join(lineas), encoding="utf-8")
    pal = [(255, 0, 255)] + [tuple(int(c[i:i + 2], 16) for i in (1, 3, 5)) for c in PALETA_OBJETOS]
    vista = Image.new("RGB", (len(todas) * 18, 16), (90, 160, 200))
    for k, m in enumerate(todas):
        for y in range(16):
            for x in range(16):
                if m[y, x]:
                    vista.putpixel((k * 18 + x, y), pal[m[y, x]])
    vista.resize((vista.width * 4, 64), Image.NEAREST).save(DESTINO / "objetos_vista.png")
    print(f"objetos: {len(todas)} piezas, {len(todas) * 128} bytes")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
