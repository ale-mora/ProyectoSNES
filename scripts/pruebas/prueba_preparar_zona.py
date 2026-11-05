"""
prueba_preparar_zona.py — Pruebas de scripts/preparar_zona.py.

Se ejecutan desde la raíz del repositorio:
    python -m unittest scripts/pruebas/prueba_preparar_zona.py

Comprueban limitar_tiles (la lejana no puede pasar de LIMITE_LEJOS tiles distintos: junta
los menos usados con el más parecido, contando los volteos como iguales, igual que gfx4snes)
y que las zonas configuradas existen.
"""

import sys
import unittest
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import preparar_zona as pz  # noqa: E402


def distintos(ind):
    s = set()
    for y in range(0, ind.shape[0], 8):
        for x in range(0, ind.shape[1], 8):
            t = ind[y:y + 8, x:x + 8]
            s.add(min(t.tobytes(), t[:, ::-1].tobytes(), t[::-1].tobytes(), t[::-1, ::-1].tobytes()))
    return len(s)


class PruebaLimitar(unittest.TestCase):
    def test_baja_hasta_el_limite(self):
        rng = np.random.default_rng(3)
        ind = rng.integers(0, 16, size=(64, 64), dtype=np.uint8)   # 64 tiles distintos.
        self.assertEqual(distintos(ind), 64)
        out = pz.limitar_tiles(ind.copy(), 40)
        self.assertEqual(distintos(out), 40)

    def test_no_toca_si_cabe(self):
        ind = np.zeros((16, 16), np.uint8)
        ind[0:8, 0:8] = 3
        out = pz.limitar_tiles(ind.copy(), 10)
        self.assertTrue((out == ind).all())

    def test_volteos_cuentan_como_uno(self):
        t = np.arange(64, dtype=np.uint8).reshape(8, 8) % 16
        ind = np.concatenate([t, t[:, ::-1]], axis=1)              # Un tile y su espejo.
        self.assertEqual(distintos(ind), 1)


class PruebaConfiguracion(unittest.TestCase):
    def test_tres_zonas_de_1024(self):
        self.assertEqual(set(pz.ZONAS), {"bosque", "monasterio", "dojo"})
        self.assertEqual(pz.ANCHO, 1024)
        self.assertEqual(pz.COLUMNAS, 128)


if __name__ == "__main__":
    unittest.main()
