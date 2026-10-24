"""
prueba_piezas16.py — Pruebas de scripts/piezas16.py.

Se ejecutan desde la raíz del repositorio:
    python -m unittest scripts/pruebas/prueba_piezas16.py

Comprueban el formato .p16 que leen data.asm y src/juego/subida.c: el índice de
8 bytes por cuadro, las tablas (dx, dy) en orden de lectura y los tiles en el
orden de subida (pares completos tal como van en la VRAM; el último par a medias
como su fila de arriba y luego la de abajo), con n·128 bytes por cuadro.
"""

import struct
import sys
import unittest
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import piezas16 as p16  # noqa: E402


def cuadro_con_piezas(n, ancho=160, alto=16):
    """Un cuadro indexado con n piezas llenas en la fila de arriba; la pieza k tiene
    el color k+1 en su píxel de arriba a la izquierda (para reconocerla)."""
    img = Image.new("P", (ancho, alto), 0)
    for k in range(n):
        img.putpixel((k * 16, 0), (k % 15) + 1)
    return img


class PruebaTiles(unittest.TestCase):
    def test_tile_4bpp(self):
        ind = [0] * 64
        ind[0], ind[7] = 1, 8
        t = p16.tile_4bpp(ind)
        self.assertEqual(t[0], 0x80)
        self.assertEqual(t[17], 0x01)

    def test_orden_de_subida_par_completo_y_medio(self):
        # 10 piezas: un par completo (1 KB) y uno a medias con 2 piezas (2·64 + 2·64).
        img = cuadro_con_piezas(10)
        piezas = p16.piezas_no_vacias(img)
        self.assertEqual(len(piezas), 10)
        datos = p16.tiles_en_orden_de_subida(img, piezas)
        self.assertEqual(len(datos), 10 * 128)
        # Pieza 1 (color 2: solo el plano 1): su tile de arriba a la izquierda es el
        # tile 2 de la fila de arriba del par.
        self.assertEqual(datos[2 * 32], 0x00)
        self.assertEqual(datos[2 * 32 + 1], 0x80)
        # Pieza 8 (la primera del par a medias): su tile de arriba empieza en el byte 1024.
        self.assertEqual(datos[1024], 0x80)                # color 9 = planos 0 y 3
        self.assertEqual(datos[1024 + 17], 0x80)
        # Fila de abajo del par a medias: tras los 4 tiles de arriba (128 B); vacía.
        self.assertEqual(datos[1024 + 128:1024 + 256], bytes(128))


class PruebaFormato(unittest.TestCase):
    def test_indice_y_tablas(self):
        cuadros = [cuadro_con_piezas(3), cuadro_con_piezas(0), cuadro_con_piezas(9)]
        datos, listas = p16.construir_p16(cuadros)
        self.assertEqual([len(l) for l in listas], [3, 0, 9])
        regs = [struct.unpack_from("<HHBBH", datos, i * 8) for i in range(3)]
        # Tablas justo detrás del índice (3 · 8 = 24), en orden; cada cuadro, su tabla
        # y detrás la espejada (n pares cada una).
        self.assertEqual(regs[0][0], 24)
        self.assertEqual(regs[0][4], 24 + 3 * 2)
        self.assertEqual(regs[1][0], 24 + 3 * 4)
        self.assertEqual(regs[2][0], 24 + 3 * 4)
        self.assertEqual(regs[2][4], 24 + 3 * 4 + 9 * 2)
        self.assertEqual([r[2] for r in regs], [3, 0, 9])
        # Los tiles empiezan en dirección par y cada cuadro ocupa n·128.
        self.assertEqual(regs[0][1] % 2, 0)
        self.assertEqual(regs[2][1] - regs[0][1], 3 * 128)
        self.assertEqual(len(datos), regs[0][1] + 12 * 128)
        # La tabla del cuadro 2 lista sus piezas (dx, dy) en orden de lectura; la
        # espejada, dx' = 160 - 16 - dx (el cuadro mide 160 de ancho).
        d = regs[2][0]
        self.assertEqual(list(datos[d:d + 6]), [0, 0, 16, 0, 32, 0])
        e = regs[2][4]
        self.assertEqual(list(datos[e:e + 6]), [144, 0, 128, 0, 112, 0])

    def test_paleta(self):
        img = Image.new("P", (16, 16), 0)
        img.putpalette([0, 0, 0, 255, 0, 0] + [0] * 762)
        pal = p16.paleta_bgr555(img)
        self.assertEqual(len(pal), 32)
        self.assertEqual(pal[2:4], bytes((0x1F, 0x00)))


if __name__ == "__main__":
    unittest.main()
