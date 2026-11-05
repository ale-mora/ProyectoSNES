"""
prueba_preparar_objetos.py — Pruebas de scripts/preparar_objetos.py.

Se ejecutan desde la raíz del repositorio:
    python -m unittest scripts/pruebas/prueba_preparar_objetos.py

Comprueban el orden de las piezas (un objeto de 32x32 son 4: TL, TR, BL, BR) y el de
los tiles dentro de cada pieza (arriba TL, TR; abajo BL, BR), que es el que copia
objetos_sprite.c a la VRAM.
"""

import sys
import unittest
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import preparar_objetos as po  # noqa: E402
from preparar_aldea import PALETA_OBJETOS, hex_rgb  # noqa: E402


class PruebaPiezas(unittest.TestCase):
    def test_32x32_son_cuatro_piezas_en_orden(self):
        a = np.zeros((32, 32, 4), np.uint8)
        a[0, 0] = list(hex_rgb(PALETA_OBJETOS[0])) + [255]     # TL
        a[0, 16] = list(hex_rgb(PALETA_OBJETOS[1])) + [255]    # TR
        a[16, 0] = list(hex_rgb(PALETA_OBJETOS[2])) + [255]    # BL
        a[16, 16] = list(hex_rgb(PALETA_OBJETOS[3])) + [255]   # BR
        p = po.piezas_de(Image.fromarray(a, "RGBA"))
        self.assertEqual([m[0, 0] for m in p], [1, 2, 3, 4])

    def test_bytes_de_una_pieza(self):
        m = np.zeros((16, 16), np.uint8)
        m[0, 8] = 1                                             # Tile TR, píxel (0, 0).
        m[8, 0] = 2                                             # Tile BL, píxel (0, 0).
        b = po.bytes_pieza(m)
        self.assertEqual(len(b), 128)
        self.assertEqual(b[32] & 0x80, 0x80)                    # TR: el segundo tile, plano 0.
        self.assertEqual(b[64 + 1] & 0x80, 0x80)                # BL: el tercero, plano 1.


if __name__ == "__main__":
    unittest.main()
