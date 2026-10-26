#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    crear_zarza_ejemplo.py — Fabrica una zarza.png de 16x16 de EJEMPLO para el
    ejercicio B de la unidad 19, mientras no llegue la de Pixellab.

    PROPÓSITO
      El ejercicio pide integrar una pieza que hoy es un marcador (la zarza).
      La pieza de verdad hay que pedirla a Pixellab (lista en
      res/descargas/graficos/pixellab/suelo/README.md). Para poder practicar ya,
      este script dibuja una a mano, píxel a píxel, con las reglas que pide ese
      README: 16x16, fondo transparente sin semitransparencias, una mata baja
      (unos 12 px) que se repite en horizontal. NO es arte de Pixellab: es un
      sustituto de programador. Usa a propósito verdes que no están en la
      paleta del nivel, para ver qué hace con ellos preparar_nivel.py.

    SE CONECTA CON
      Escribe:  <carpeta>/zarza.png (por defecto res/descargas/graficos/pixellab/suelo/).
      Lo usa:   scripts/preparar_nivel.py (con la solución del ejercicio B).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES
      DIBUJO   list[str]  16 filas de 16 caracteres: '.' transparente, letra = color
      COLORES  dict       letra -> (r, g, b)

    Uso (desde la raíz del repo o del clon):
      python aprendizaje/unidad-19-del-arte-al-juego/solucion/crear_zarza_ejemplo.py
---------------------------------------------------------------------------------
"""
import sys
from pathlib import Path

from PIL import Image

# Una mata de 12 px de alto (filas 4..15) con espinas claras; los bordes izquierdo
# y derecho encajan para que la fila de zarzas no tenga costura.
DIBUJO = [
    "................",
    "................",
    "................",
    "................",
    "....e......e....",
    "...eo.....oe....",
    "..ooo.e..ooo.e..",
    ".oOoo.eo.oOooeo.",
    "ooOOoooooOOoooOo",
    "oOOdOoeoOOdOoOOo",
    "OOddOOoOOddOOdOO",
    "OdddOdOOdddOddOd",
    "dddddddddddddddd",
    "ddtdddddddtddddd",
    "dtdtddtdddtdtddt",
    "tttttttttttttttt",
]
COLORES = {
    "e": (232, 226, 170),   # espina (clara)
    "o": (96, 156, 60),     # hoja clara
    "O": (58, 112, 44),     # hoja media
    "d": (34, 70, 34),      # hoja oscura
    "t": (88, 56, 30),      # tallo, junto a la tierra
}


def crear(carpeta: Path) -> Path:
    """Dibuja la zarza y la guarda como zarza.png en `carpeta`."""
    im = Image.new("RGBA", (16, 16), (0, 0, 0, 0))
    for y, fila in enumerate(DIBUJO):
        for x, letra in enumerate(fila):
            if letra != ".":
                im.putpixel((x, y), COLORES[letra] + (255,))
    carpeta.mkdir(parents=True, exist_ok=True)
    ruta = carpeta / "zarza.png"
    im.save(ruta)
    return ruta


if __name__ == "__main__":
    destino = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("res/descargas/graficos/pixellab/suelo")
    print(f"  {crear(destino)}  16x16, {len(COLORES)} colores (ejemplo, no es de Pixellab)")
