#!/usr/bin/env python3
"""
generar_fuente_consola.py — Convierte Ark Pixel Font (10 px, latín) en la fuente
bitmap que usa la consola de texto de PVSnesLib.

Formato de salida (el mismo que `pvsneslibfont.bmp` del SDK):
  - BMP indexado de 768x8 píxeles: 96 celdas de 8x8, una por carácter ASCII
    del 32 (espacio) al 127, en orden.
  - Índice 0 = fondo (magenta, transparente en la SNES); índice 1 = trazo (blanco).
  - El glifo ocupa las filas 0..6; la fila 7 queda vacía para separar líneas.

Ark Pixel 10 px: las mayúsculas y los dígitos miden 7x7 y caben en 8x8; las
minúsculas tienen descendentes de 9 px y no caben, así que se dibujan con el
glifo de su mayúscula (estilo de época: todo en mayúsculas).

Uso (desde la raíz del repo):
  python scripts/generar_fuente_consola.py
Salida: res/fuente/ark_consola.bmp
"""

from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

FUENTE = Path("res/descargas/fuentes/ark-pixel-font-10px-proportional-ttf-v2026.09.01/ark-pixel-10px-proportional-latin.ttf")
SALIDA = Path("res/fuente/ark_consola.bmp")
TAMANO = 10
# Fila donde Ark Pixel 10px apoya la parte alta de las mayúsculas cuando se
# dibuja en (0, 0): medido con Pillow, las mayúsculas van de la fila 4 a la 10.
FILA_TOPE_MAYUSCULAS = 4
ANCHO_CELDA = 8
CARACTERES = 96          # ASCII 32..127


def glifo(fuente: ImageFont.FreeTypeFont, caracter: str) -> Image.Image:
    """Devuelve el carácter dibujado en un lienzo de 8x8 (modo 'L', 0 = fondo)."""
    lienzo = Image.new("L", (32, 32), 0)
    ImageDraw.Draw(lienzo).text((4, 4), caracter, font=fuente, fill=255)
    # Recortamos la ventana de 8x8 cuyo borde superior es el tope de mayúsculas:
    # así las letras quedan en las filas 0..6 y la fila 7 queda libre.
    x0 = 4
    y0 = 4 + FILA_TOPE_MAYUSCULAS
    celda = lienzo.crop((x0, y0, x0 + ANCHO_CELDA, y0 + ANCHO_CELDA))
    # Umbral: Pillow puede antialiasar; todo lo que no sea negro es trazo.
    return celda.point(lambda v: 255 if v > 64 else 0)


def main() -> int:
    if not FUENTE.exists():
        raise SystemExit(f"No se encuentra la fuente: {FUENTE}")
    fuente = ImageFont.truetype(str(FUENTE), TAMANO)

    hoja = Image.new("P", (ANCHO_CELDA * CARACTERES, 8), 0)
    paleta = [255, 0, 255, 255, 255, 255] + [0, 0, 0] * 254   # idx0 magenta, idx1 blanco
    hoja.putpalette(paleta)

    recortados = []
    for i in range(CARACTERES):
        codigo = 32 + i
        caracter = chr(codigo)
        if "a" <= caracter <= "z":
            caracter = caracter.upper()          # minúsculas → mayúsculas
        if codigo == 127:
            caracter = " "                       # DEL no tiene dibujo
        celda = glifo(fuente, caracter)
        # Volcamos la celda: cada píxel blanco pasa a índice 1.
        for y in range(8):
            for x in range(ANCHO_CELDA):
                if celda.getpixel((x, y)):
                    hoja.putpixel((i * ANCHO_CELDA + x, y), 1)
        # Aviso si el glifo se salió de la celda (lo comprobamos en el lienzo grande).
        lienzo = Image.new("L", (32, 32), 0)
        ImageDraw.Draw(lienzo).text((4, 4), caracter, font=fuente, fill=255)
        bb = lienzo.getbbox()
        if bb and (bb[2] - 4 > ANCHO_CELDA or bb[3] - 4 - FILA_TOPE_MAYUSCULAS > 7 or bb[1] - 4 < FILA_TOPE_MAYUSCULAS):
            recortados.append(chr(codigo))

    SALIDA.parent.mkdir(parents=True, exist_ok=True)
    hoja.save(SALIDA, format="BMP")
    print(f"Fuente generada: {SALIDA} ({hoja.size[0]}x{hoja.size[1]}, modo {hoja.mode})")
    if recortados:
        print("Caracteres recortados por no caber en 8x8 (revisar a ojo):", "".join(recortados))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
