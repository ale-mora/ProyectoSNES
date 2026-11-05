"""
prueba_preparar_aldea.py — Pruebas de scripts/preparar_aldea.py.

Se ejecutan desde la raíz del repositorio:
    python -m unittest scripts/pruebas/prueba_preparar_aldea.py

Comprueban lo que la SNES necesita que esté bien sin mirar el arte: la palabra del
mapa (tile, paleta, prioridad, volteos), que el juego de tiles no repite tiles (ni
volteados) y no comparte los reservados del noren, que la reducción respeta el
límite, los fijos y la paleta, el mapa columna a columna, la elección de paleta por
tile, la calle sin columna de marco, la costura de la lejana tapada y el cielo con
un color por línea.
"""

import struct
import sys
import unittest
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import preparar_aldea as al  # noqa: E402


def tile(valores):
    """Un tile de 64 índices a partir de una lista (se repite si es corta)."""
    v = (list(valores) * 64)[:64]
    return np.array(v, np.uint8)


class PruebaMapa(unittest.TestCase):
    def test_entrada(self):
        self.assertEqual(al.entrada(0x1FF, 5, 1, 1, 0), 0x4000 | 0x2000 | (5 << 10) | 0x1FF)
        self.assertEqual(al.entrada(3, 4, 0, 0, 1), 0x8000 | (4 << 10) | 3)

    def test_mapa_columna_a_columna(self):
        mapa = [[(cy * 1000 + cx, 0, 0, 0, 0) for cx in range(al.COLUMNAS)] for cy in range(al.FILAS)]
        datos = al.mapa_columnas(mapa)
        self.assertEqual(len(datos), al.COLUMNAS * al.FILAS * 2)
        vals = struct.unpack(f"<{al.COLUMNAS * al.FILAS}H", datos)
        base = al.PALETA_SNES_ARQ << 10
        self.assertEqual(vals[0], base | 0)                       # (col 0, fila 0)
        self.assertEqual(vals[1], base | 1000)                    # (col 0, fila 1)
        self.assertEqual(vals[al.FILAS], base | 1)                # (col 1, fila 0)

    def test_paleta_de_los_aldeanos(self):
        mapa = [[(0, 1, 0, 0, 0)] * al.COLUMNAS for _ in range(al.FILAS)]
        vals = struct.unpack("<2H", al.mapa_columnas(mapa)[:4])
        self.assertEqual(vals[0] >> 10 & 7, al.PALETA_SNES_GENTE)


class PruebaJuego(unittest.TestCase):
    def test_vacio_es_el_tile_0(self):
        j = al.Juego()
        self.assertEqual(j.buscar(np.zeros(64, np.uint8), 0), (0, 0, 0))
        self.assertEqual(len(j.tiles), 1)

    def test_volteados_son_el_mismo(self):
        j = al.Juego()
        m = np.arange(64, dtype=np.uint8) % 15 + 1
        t, h, v = j.buscar(m, 0)
        self.assertEqual((h, v), (0, 0))
        espejo = m.reshape(8, 8)[:, ::-1].reshape(64)
        self.assertEqual(j.buscar(espejo, 0), (t, 1, 0))
        boca_abajo = m.reshape(8, 8)[::-1, :].reshape(64)
        self.assertEqual(j.buscar(boca_abajo, 0), (t, 0, 1))
        self.assertEqual(len(j.tiles), 2)

    def test_otra_paleta_es_otro_tile(self):
        j = al.Juego()
        a = j.buscar(tile([1, 2]), 0)[0]
        b = j.buscar(tile([1, 2]), 1)[0]
        self.assertNotEqual(a, b)

    def test_reservado_no_se_comparte(self):
        j = al.Juego()
        r = j.reservar(tile([3]), 0)
        self.assertNotEqual(j.buscar(tile([3]), 0)[0], r)


class PruebaReducir(unittest.TestCase):
    def test_junta_los_parecidos_hasta_el_limite(self):
        j = al.Juego()
        fijo = j.reservar(tile([9]), 0)
        a = j.buscar(tile([1]), 0)[0]
        casi_a = tile([1]); casi_a[0] = 2                        # Se parece a `a`.
        b = j.buscar(casi_a, 0)[0]
        c = j.buscar(tile([5, 6, 7]), 0)[0]                      # Muy distinto.
        mapa = [[(a, 0, 0, 0, 0), (b, 0, 0, 0, 0), (c, 0, 0, 0, 0), (fijo, 0, 0, 0, 0)]]
        juntados = al.reducir(j, mapa, limite=4, fijos=2)
        self.assertEqual(juntados, 1)
        self.assertEqual(len(j.tiles), 4)
        self.assertEqual(mapa[0][0][0], mapa[0][1][0])           # a y casi_a: uno solo.
        self.assertNotEqual(mapa[0][2][0], mapa[0][0][0])
        self.assertEqual(mapa[0][3][0], fijo)                    # El reservado, intacto.

    def test_no_junta_paletas_distintas(self):
        j = al.Juego()
        a = j.buscar(tile([1]), 0)[0]
        b = j.buscar(tile([1]), 1)[0]
        c = j.buscar(tile([4, 5]), 0)[0]
        mapa = [[(a, 0, 0, 0, 0), (b, 1, 0, 0, 0), (c, 0, 0, 0, 0)]]
        al.reducir(j, mapa, limite=3, fijos=1)
        self.assertEqual(mapa[0][0][0], mapa[0][2][0])           # Juntó a con c (paleta 0)...
        self.assertNotEqual(mapa[0][1][0], mapa[0][0][0])        # ...y no con b.

    def test_dentro_del_limite_no_hace_nada(self):
        j = al.Juego()
        j.buscar(tile([1]), 0)
        self.assertEqual(al.reducir(j, [[]], limite=10, fijos=1), 0)


class PruebaPiezas(unittest.TestCase):
    def test_tile_de_elige_la_paleta_que_lo_dibuja(self):
        img = np.zeros((8, 8, 4), np.uint8)
        img[...] = list(al.hex_rgb(al.PALETA_GENTE[1])) + [255]   # Piel: solo en la de gente.
        ind, pal = al.tile_de(img, 0, 0, [al.PALETA_ARQ, al.PALETA_GENTE])
        self.assertEqual(pal, 1)
        self.assertTrue((ind == 2).all())                        # Color 1 de la lista = índice 2.

    def test_transparente_es_indice_0(self):
        pix = np.zeros((64, 4), np.uint8)
        ind, err = al.indices_tile(pix, al.PALETA_ARQ)
        self.assertFalse(ind.any())
        self.assertEqual(err, 0)

    def test_calle_sin_columna_de_marco(self):
        a = np.zeros((4, 8, 4), np.uint8)
        a[:, -2] = (10, 20, 30, 255)
        a[:, -1] = (250, 250, 250, 255)
        c = np.array(al.preparar_calle(Image.fromarray(a, "RGBA")))
        self.assertTrue((c[:, -1] == c[:, -2]).all())

    def test_costura_tapada(self):
        img = Image.new("RGBA", (512, 160), (0, 0, 0, 0))
        a = np.array(img)
        a[60:, :256] = (255, 0, 0, 255)                          # Dos mitades que no casan.
        a[100:, 256:] = (0, 0, 255, 255)
        b = np.array(al.tapar_costura(Image.fromarray(a, "RGBA")))
        oscuro = list(al.hex_rgb(al.PALETA_LEJOS[6])) + [255]
        for x in (0, 511):                                       # La columna de la costura y
            self.assertTrue((b[60:, x] == oscuro).all())         # la de al lado: el monte.
        self.assertTrue((b[150, 256] == (0, 0, 255, 255)).all())  # Lejos de ella, igual.

    def test_cielo_un_color_por_linea(self):
        img = al.cielo().convert("RGB")
        self.assertEqual(img.size, (256, 224))
        a = np.array(img)
        self.assertTrue((a == a[:, :1]).all())                   # Cada línea, de un color.
        arriba, abajo = a[0, 0], a[223, 0]
        self.assertGreater(int(arriba[2]), int(arriba[1]))       # Arriba, violeta (más azul).
        self.assertGreater(int(abajo[0]), int(abajo[2]))         # Abajo, naranja.


if __name__ == "__main__":
    unittest.main()
