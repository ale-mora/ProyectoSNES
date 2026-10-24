"""
prueba_preparar_fondo_nivel.py — Pruebas de scripts/preparar_fondo_nivel.py.

Se ejecutan desde la raíz del repositorio:
    python -m unittest scripts/pruebas/prueba_preparar_fondo_nivel.py

El script toma la capa lejana del título (256 px, indexada, con el logotipo
horneado), borra una caja (la pone al índice 0, transparente) y duplica la
imagen en espejo hasta 512 px conservando la paleta, como pide
docs/diseno-demo-jugable.md §8.
"""

import sys
import tempfile
import unittest
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import preparar_fondo_nivel as pfn  # noqa: E402


def imagen_indexada(ancho=64, alto=32) -> Image.Image:
    """Imagen P con índice 0 arriba (cielo), un "logo" de índice 3 en (10..20, 4..8)
    y montañas de índice 2 en la mitad inferior, con una columna de índice 1 en x=5."""
    im = Image.new("P", (ancho, alto), 0)
    im.putpalette([0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255] + [0, 0, 0] * 252)
    px = im.load()
    for y in range(alto // 2, alto):
        for x in range(ancho):
            px[x, y] = 2
    for y in range(4, 8):
        for x in range(10, 20):
            px[x, y] = 3
    for y in range(alto // 2, alto):
        px[5, y] = 1
    return im


class PruebasPrepararFondoNivel(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.entrada = Path(self.tmp.name) / "lejos.png"
        self.salida = Path(self.tmp.name) / "nivel" / "fondo_lejos.png"
        imagen_indexada().save(self.entrada)

    def tearDown(self):
        self.tmp.cleanup()

    def test_doble_de_ancho_y_espejo(self):
        pfn.preparar(self.entrada, self.salida, borrar=(10, 4, 20, 8))
        with Image.open(self.salida) as im:
            self.assertEqual(im.size, (128, 32))
            self.assertEqual(im.mode, "P")
            px = im.load()
            self.assertEqual(px[5, 20], 1)           # columna original en x=5
            self.assertEqual(px[128 - 1 - 5, 20], 1) # y su espejo en x=122
            self.assertEqual(px[0, 20], 2)
            self.assertEqual(px[127, 20], 2)

    def test_logo_borrado_en_ambas_mitades(self):
        pfn.preparar(self.entrada, self.salida, borrar=(10, 4, 20, 8))
        with Image.open(self.salida) as im:
            px = im.load()
            for x in range(10, 20):
                for y in range(4, 8):
                    self.assertEqual(px[x, y], 0)
                    self.assertEqual(px[128 - 1 - x, y], 0)

    def test_relleno_de_alto(self):
        """gfx4snes -y exige un alto múltiplo de 256: con alto=64 la salida crece
        hasta 64 filas y las filas nuevas quedan en el índice 0 (transparente)."""
        pfn.preparar(self.entrada, self.salida, borrar=(10, 4, 20, 8), alto=64)
        with Image.open(self.salida) as im:
            self.assertEqual(im.size, (128, 64))
            px = im.load()
            self.assertEqual(px[5, 20], 1)             # lo de arriba no se mueve
            for y in range(32, 64):
                self.assertEqual(px[5, y], 0)
                self.assertEqual(px[127, y], 0)

    def test_paleta_conservada(self):
        pfn.preparar(self.entrada, self.salida, borrar=(10, 4, 20, 8))
        with Image.open(self.entrada) as a, Image.open(self.salida) as b:
            self.assertEqual(a.getpalette()[:48], b.getpalette()[:48])


if __name__ == "__main__":
    unittest.main()
