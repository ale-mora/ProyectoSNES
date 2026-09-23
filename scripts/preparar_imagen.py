#!/usr/bin/env python3
"""
preparar_imagen.py — Convierte una imagen cualquiera (PNG, JPG, BMP, GIF...) a
un PNG **indexado** listo para `gfx4snes`, la herramienta de PVSnesLib que
transforma imágenes en tiles/paletas/mapas para la SNES.

Qué hace, en orden:
  1. Abre la imagen y la pasa a RGB (quita transparencia, CMYK, etc.).
  2. Si se piden --ancho/--alto, la escala para cubrir ese tamaño y recorta
     el sobrante centrado (o la rellena con negro si es más chica).
  3. Reduce la cantidad de colores a --colores (16 para fondos/sprites de
     Mode 1, 256 para Mode 7 / Mode 3).
  4. Guarda un PNG en modo "P" (paleta) con 256 entradas, que es exactamente
     lo que gfx4snes espera.

Uso:
  python scripts/preparar_imagen.py ENTRADA SALIDA [--colores 16|256]
                                     [--ancho W --alto H] [--sin-recorte]

Ejemplos:
  python scripts/preparar_imagen.py res/descargas/graficos/pueblo.jpg res/intro/escena_1.png --colores 16 --ancho 256 --alto 224
  python scripts/preparar_imagen.py res/descargas/graficos/logo.png res/devs/logo_devs.png --colores 256 --ancho 256 --alto 224

Notas SNES:
  - En 16 colores, el color de índice 0 es el "transparente" del tile; en un
    fondo a pantalla completa no importa, en un sprite conviene que el color 0
    sea el fondo que no debe verse.
  - Tras convertir, `gfx4snes` puede quejarse si un tile de 8x8 mezcla más de
    16 colores distintos: bajar --colores o simplificar la imagen resuelve.
"""

import argparse
import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:  # pragma: no cover
    sys.exit("Falta Pillow. Instalar con: python -m pip install pillow")


def ajustar_tamano(imagen: Image.Image, ancho: int, alto: int, recortar: bool) -> Image.Image:
    """Escala y recorta (o rellena) la imagen para que quede exactamente ancho x alto."""
    if recortar:
        # Escalar para CUBRIR el rectángulo destino (puede sobrar en un eje)...
        escala = max(ancho / imagen.width, alto / imagen.height)
    else:
        # ...o para CABER dentro (puede faltar en un eje, se rellena con negro).
        escala = min(ancho / imagen.width, alto / imagen.height)

    nuevo_ancho = max(1, round(imagen.width * escala))
    nuevo_alto = max(1, round(imagen.height * escala))
    # NEAREST conserva el pixel art nítido; LANCZOS suaviza fotos. Elegimos
    # según si estamos agrandando (pixel art) o achicando (fotos/renders).
    filtro = Image.Resampling.NEAREST if escala >= 1 else Image.Resampling.LANCZOS
    imagen = imagen.resize((nuevo_ancho, nuevo_alto), filtro)

    lienzo = Image.new("RGB", (ancho, alto), (0, 0, 0))
    desplazamiento = ((ancho - nuevo_ancho) // 2, (alto - nuevo_alto) // 2)
    lienzo.paste(imagen, desplazamiento)
    return lienzo


def indexar(imagen: Image.Image, colores: int) -> Image.Image:
    """Reduce a `colores` colores y devuelve una imagen en modo paleta ("P")."""
    # MEDIANCUT da buenos resultados con pocos colores; dither NONE evita el
    # "ruido" punteado que arruina el pixel art y multiplica tiles únicos.
    indexada = imagen.quantize(
        colors=colores,
        method=Image.Quantize.MEDIANCUT,
        dither=Image.Dither.NONE,
    )
    # gfx4snes quiere una paleta de 256 entradas aunque usemos 16: rellenamos
    # el resto con negro para que el archivo sea "de 256 colores".
    paleta = indexada.getpalette() or []
    paleta = paleta[: colores * 3] + [0] * (768 - colores * 3)
    indexada.putpalette(paleta)
    return indexada


def main() -> int:
    parser = argparse.ArgumentParser(description="Prepara una imagen para gfx4snes (PNG indexado).")
    parser.add_argument("entrada", type=Path, help="imagen de origen (cualquier formato que abra Pillow)")
    parser.add_argument("salida", type=Path, help="PNG de destino, indexado")
    parser.add_argument("--colores", type=int, default=16, choices=[4, 16, 256],
                        help="cantidad de colores: 4 (BG3), 16 (Mode 1, sprites) o 256 (Mode 7 / Mode 3)")
    parser.add_argument("--ancho", type=int, help="ancho final en píxeles (ej. 256)")
    parser.add_argument("--alto", type=int, help="alto final en píxeles (ej. 224)")
    parser.add_argument("--sin-recorte", action="store_true",
                        help="en vez de recortar para cubrir, encajar completa y rellenar con negro")
    args = parser.parse_args()

    if (args.ancho is None) != (args.alto is None):
        parser.error("--ancho y --alto van juntos")

    if not args.entrada.exists():
        sys.exit(f"No existe la entrada: {args.entrada}")

    imagen = Image.open(args.entrada).convert("RGB")
    print(f"Entrada : {args.entrada}  {imagen.width}x{imagen.height}")

    if args.ancho:
        imagen = ajustar_tamano(imagen, args.ancho, args.alto, recortar=not args.sin_recorte)

    if imagen.width % 8 or imagen.height % 8:
        print(f"AVISO: {imagen.width}x{imagen.height} no es múltiplo de 8; gfx4snes trabaja en tiles de 8x8.")

    indexada = indexar(imagen, args.colores)
    args.salida.parent.mkdir(parents=True, exist_ok=True)
    indexada.save(args.salida, format="PNG", optimize=False)

    # getcolors() devuelve una lista (cantidad, índice) por cada color realmente usado.
    usados = len(indexada.getcolors() or [])
    print(f"Salida  : {args.salida}  {indexada.width}x{indexada.height}  modo={indexada.mode}  colores usados={usados}/{args.colores}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
