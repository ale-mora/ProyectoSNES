"""
prueba_27_tiles.py — Pruebas de la parte A del ejercicio 27 (solución).

Se ejecutan desde la carpeta de la unidad:
    python -m unittest prueba_27_tiles            (contra tu ejercicio_27_tiles.py)
    python -m unittest solucion.prueba_27_tiles   (contra la solución)

Comprueban lo que el límite de 512 tiles necesita: el vacío es siempre el 0, un tile
repetido no se guarda dos veces, un tile espejado (en horizontal, en vertical o en los
dos) es el mismo con su bit de volteo, la paleta distingue, y los volteos son exactos
(un tile simétrico no se confunde).
"""

import importlib
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
et = importlib.import_module("ejercicio_27_tiles")


def tile(filas):
    """8 cadenas de 8 dígitos hexadecimales → tupla de 64 índices."""
    return tuple(int(c, 16) for fila in filas for c in fila)


ESQUINA = tile(["10000000"] + ["00000000"] * 7)          # Un píxel arriba a la izquierda.
DIAGONAL = tile(["12000000", "01200000", "00120000", "00012000",
                 "00001200", "00000120", "00000012", "00000001"])


class PruebaVolteos(unittest.TestCase):
    def test_horizontal(self):
        self.assertEqual(et.voltear_h(ESQUINA), tile(["00000001"] + ["00000000"] * 7))

    def test_vertical(self):
        self.assertEqual(et.voltear_v(ESQUINA), tile(["00000000"] * 7 + ["10000000"]))

    def test_dos_veces_es_el_mismo(self):
        self.assertEqual(et.voltear_h(et.voltear_h(DIAGONAL)), DIAGONAL)
        self.assertEqual(et.voltear_v(et.voltear_v(DIAGONAL)), DIAGONAL)


class PruebaJuego(unittest.TestCase):
    def test_vacio_es_el_0(self):
        j = et.Juego()
        self.assertEqual(j.buscar(et.VACIO, 3), (0, 0, 0))
        self.assertEqual(len(j.tiles), 1)

    def test_repetido(self):
        j = et.Juego()
        a = j.buscar(DIAGONAL, 0)
        self.assertEqual(a, (1, 0, 0))
        self.assertEqual(j.buscar(DIAGONAL, 0), (1, 0, 0))
        self.assertEqual(len(j.tiles), 2)

    def test_volteados(self):
        j = et.Juego()
        j.buscar(ESQUINA, 0)
        self.assertEqual(j.buscar(et.voltear_h(ESQUINA), 0), (1, 1, 0))
        self.assertEqual(j.buscar(et.voltear_v(ESQUINA), 0), (1, 0, 1))
        self.assertEqual(j.buscar(et.voltear_h(et.voltear_v(ESQUINA)), 0), (1, 1, 1))
        self.assertEqual(len(j.tiles), 2)

    def test_el_volteo_es_respecto_al_guardado(self):
        j = et.Juego()
        j.buscar(et.voltear_h(ESQUINA), 0)                # Se guarda el espejado...
        self.assertEqual(j.buscar(ESQUINA, 0), (1, 1, 0))  # ...y el original es su volteo.

    def test_la_paleta_distingue(self):
        j = et.Juego()
        self.assertNotEqual(j.buscar(DIAGONAL, 0)[0], j.buscar(DIAGONAL, 1)[0])

    def test_simetrico(self):
        j = et.Juego()
        simetrico = tile(["01111110"] * 8)               # Igual a su espejo horizontal.
        self.assertEqual(j.buscar(simetrico, 0), (1, 0, 0))
        self.assertEqual(j.buscar(et.voltear_h(simetrico), 0), (1, 0, 0))   # Sin volteo.


if __name__ == "__main__":
    unittest.main()
