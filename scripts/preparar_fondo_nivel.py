#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    preparar_fondo_nivel.py — Deriva la capa lejana del nivel 1 a partir de la del
    título: borra el logotipo horneado y duplica la imagen en espejo hasta 512 px.

    PROPÓSITO
      El título usa res/titulo/fondo_lejos.png (256 px, 16 colores, cielo
      transparente) con el logotipo dibujado encima. El nivel necesita la misma
      montaña sin logotipo y de 512 px de ancho (mapa SC_64x32 para el scroll de
      parallax). Original + espejo horizontal da un bucle sin costura, el mismo
      truco que la capa cercana de arbustos.

    SE CONECTA CON
      Lee:      res/titulo/fondo_lejos.png (indexada, generada por componer_escenas.py).
      Escribe:  res/nivel1/fondo_lejos.png (indexada, misma paleta, 512x224).
      Lo usan:  el Makefile (gfx4snes -y ... res/nivel1/fondo_lejos.pic) y data.asm
                (sección .rodata_nivel1_lejos); src/pantallas/pantalla_nivel1.c
                la carga en BG1. Pruebas: scripts/pruebas/prueba_preparar_fondo_nivel.py.

    HARDWARE QUE TOCA
      ninguno. Regla de la PPU que respeta: el índice 0 es transparente en las
      capas superiores, así que la caja borrada deja ver el cielo HDMA.

    VARIABLES (parámetros)
      entrada  Path   PNG indexado de 256 px
      salida   Path   PNG indexado del doble de ancho
      borrar   tuple  (x0, y0, x1, y1) caja que pasa al índice 0; x1,y1 exclusivos

    Uso (desde la raíz del repo):
      python scripts/preparar_fondo_nivel.py res/titulo/fondo_lejos.png res/nivel1/fondo_lejos.png --borrar 71,22,185,76
    → glosario: Tilemap, Paleta, Parallax
---------------------------------------------------------------------------------
"""

import argparse
from pathlib import Path

from PIL import Image, ImageOps


def preparar(entrada: Path, salida: Path, borrar: tuple, alto: int | None = None) -> None:
    """Borra la caja `borrar` (índice 0) y guarda original + espejo, misma paleta.
    Si `alto` es mayor que el de la imagen, rellena por abajo con índice 0:
    gfx4snes -y (páginas de 32x32 tiles) exige un alto múltiplo de 256 y aborta
    con 224 px; las filas sobrantes nunca se ven porque no hay scroll vertical."""
    with Image.open(entrada) as im:
        if im.mode != "P":
            raise SystemExit(f"ERROR: {entrada} no es una imagen indexada (modo {im.mode})")
        paleta = im.getpalette()
        base = im.copy()
    x0, y0, x1, y1 = borrar
    px = base.load()
    for y in range(y0, y1):
        for x in range(x0, x1):
            px[x, y] = 0                                   # transparente: se ve el cielo HDMA
    espejo = ImageOps.mirror(base)                         # mismo modo P, misma paleta
    alto_final = max(base.height, alto or 0)
    doble = Image.new("P", (base.width * 2, alto_final), 0)
    doble.putpalette(paleta)
    doble.paste(base, (0, 0))
    doble.paste(espejo, (base.width, 0))
    salida.parent.mkdir(parents=True, exist_ok=True)
    doble.save(salida, format="PNG")
    print(f"  {salida}  {doble.width}x{doble.height}  caja borrada {borrar}")


def main() -> int:
    ap = argparse.ArgumentParser(description="Capa lejana del nivel: sin logotipo y de 512 px.")
    ap.add_argument("entrada", type=Path)
    ap.add_argument("salida", type=Path)
    ap.add_argument("--borrar", type=str, required=True, help="caja x0,y0,x1,y1 a poner transparente")
    ap.add_argument("--alto", type=int, default=256, help="alto mínimo de la salida (gfx4snes -y exige múltiplo de 256)")
    args = ap.parse_args()
    caja = tuple(int(v) for v in args.borrar.split(","))
    if len(caja) != 4:
        raise SystemExit("ERROR: --borrar necesita cuatro números: x0,y0,x1,y1")
    preparar(args.entrada, args.salida, caja, alto=args.alto)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
