#!/usr/bin/env python3
"""
preparar_sprites.py — Convierte una tira de animación (spritesheet horizontal)
en la tira indexada que necesita `gfx4snes` para sprites de la SNES.

Qué hace:
  1. Corta la tira de entrada en `--cuadros` cuadros de igual ancho.
  2. Calcula la caja que envuelve al personaje en TODOS los cuadros (así el
     personaje no "salta" entre cuadros) y la centra en una celda de
     --celda x --celda píxeles (64x64 por defecto). Si el personaje es más
     grande que la celda, se recorta por los bordes y se avisa.
  3. Reduce a 15 colores + transparente y guarda un PNG indexado donde el
     índice 0 es el fondo transparente (regla de la SNES para sprites).
  4. Opcionalmente descarta cuadros (--usar 0,2,4) para ahorrar VRAM.

Uso (desde la raíz del repo):
  python scripts/preparar_sprites.py ENTRADA SALIDA --cuadros N [--celda 64] [--usar 0,1,2] [--suelo]

  --suelo  alinea los pies del personaje con el borde inferior de la celda en
           vez de centrarlo verticalmente (útil para que "pise" el fondo).
"""

import argparse
from pathlib import Path
from PIL import Image

TRANSPARENTE = (255, 0, 255)   # magenta: nunca aparece en el arte, marca el índice 0


def main() -> int:
    ap = argparse.ArgumentParser(description="Prepara una tira de sprites para gfx4snes.")
    ap.add_argument("entrada", type=Path)
    ap.add_argument("salida", type=Path)
    ap.add_argument("--cuadros", type=int, required=True, help="cuántos cuadros tiene la tira de entrada")
    ap.add_argument("--celda", type=int, default=64, choices=[8, 16, 32, 64], help="tamaño del sprite SNES")
    ap.add_argument("--usar", type=str, default="", help="índices de cuadros a conservar, ej. 0,2,4 (vacío = todos)")
    ap.add_argument("--suelo", action="store_true", help="alinear los pies al borde inferior de la celda")
    args = ap.parse_args()

    tira = Image.open(args.entrada).convert("RGBA")
    ancho_cuadro = tira.width // args.cuadros
    cuadros = [tira.crop((k * ancho_cuadro, 0, (k + 1) * ancho_cuadro, tira.height)) for k in range(args.cuadros)]
    if args.usar:
        cuadros = [cuadros[int(i)] for i in args.usar.split(",")]

    # Caja común: el rectángulo mínimo que contiene al personaje en todos los cuadros.
    cajas = [c.getbbox() for c in cuadros if c.getbbox()]
    L = min(b[0] for b in cajas); T = min(b[1] for b in cajas)
    R = max(b[2] for b in cajas); B = max(b[3] for b in cajas)
    ancho, alto = R - L, B - T
    if ancho > args.celda or alto > args.celda:
        print(f"AVISO: el personaje mide {ancho}x{alto} y la celda es {args.celda}; se recortan los bordes.")

    # Posición del personaje dentro de la celda.
    dx = (args.celda - ancho) // 2
    dy = (args.celda - alto) if args.suelo else (args.celda - alto) // 2

    # Armamos la tira de salida sobre fondo magenta (será el índice 0).
    salida = Image.new("RGBA", (args.celda * len(cuadros), args.celda), TRANSPARENTE + (255,))
    for k, c in enumerate(cuadros):
        recorte = c.crop((L, T, R, B))
        salida.paste(recorte, (k * args.celda + dx, dy), recorte)   # el 3.er arg. usa la transparencia

    # Reducimos a 16 colores garantizando que el magenta quede en el índice 0:
    # cuantizamos solo los píxeles opacos a 15 colores y luego componemos la paleta.
    rgb = salida.convert("RGB")
    opacos = rgb.quantize(colors=15, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    pal15 = opacos.getpalette()[:15 * 3]
    indexada = Image.new("P", rgb.size, 0)
    indexada.putpalette(list(TRANSPARENTE) + pal15 + [0, 0, 0] * 240)
    px_src = opacos.load(); px_rgb = rgb.load(); px_dst = indexada.load()
    for y in range(rgb.height):
        for x in range(rgb.width):
            if px_rgb[x, y] == TRANSPARENTE:
                px_dst[x, y] = 0
            else:
                px_dst[x, y] = px_src[x, y] + 1

    args.salida.parent.mkdir(parents=True, exist_ok=True)
    indexada.save(args.salida, format="PNG")
    print(f"Salida: {args.salida}  {indexada.size[0]}x{indexada.size[1]}  cuadros={len(cuadros)}  "
          f"personaje={ancho}x{alto}  celda={args.celda}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
