"""
prueba_preparar_prueba_carga.py — Pruebas de scripts/preparar_prueba_carga.py.

Se ejecutan desde la raíz del repositorio:
    python -m unittest scripts/pruebas/prueba_preparar_prueba_carga.py

Comprueban las tres piezas que la ROM de prueba de carga da por buenas sin
poder verlas: el formato del tile 4bpp de la SNES (planos intercalados, bit 7 =
píxel de la izquierda), el recorte en piezas de 16x16 (solo las que tienen
algún píxel, en orden de lectura) y el empaquetado en pares de filas de 1 KB
(la pieza k en el tile k/8*32 + k%8*2, como la lee src/pantallas/prueba_carga.c),
más la cuenta de ranuras por línea de las predicciones.
"""

import sys
import unittest
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import preparar_prueba_carga as pc  # noqa: E402


class PruebaTile4bpp(unittest.TestCase):
    def test_un_pixel_de_cada_plano(self):
        # Fila 0: píxel 0 con valor 1 (plano 0), píxel 1 con valor 2 (plano 1),
        # píxel 2 con valor 4 (plano 2), píxel 7 con valor 8 (plano 3).
        ind = [0] * 64
        ind[0], ind[1], ind[2], ind[7] = 1, 2, 4, 8
        t = pc.tile_4bpp(ind)
        self.assertEqual(len(t), 32)
        self.assertEqual(t[0], 0x80)    # Plano 0, fila 0: el bit 7 es el píxel de la izquierda.
        self.assertEqual(t[1], 0x40)    # Plano 1, fila 0.
        self.assertEqual(t[16], 0x20)   # Plano 2, fila 0: tras los 16 bytes de los planos 0-1.
        self.assertEqual(t[17], 0x01)   # Plano 3, fila 0: el píxel 7 es el bit 0.
        self.assertEqual(sum(t), 0x80 + 0x40 + 0x20 + 0x01)

    def test_fila_7_valor_15(self):
        ind = [0] * 64
        ind[7 * 8 + 3] = 15
        t = pc.tile_4bpp(ind)
        for byte in (14, 15, 30, 31):   # Fila 7 de los cuatro planos.
            self.assertEqual(t[byte], 0x10)

    def test_color_bgr555(self):
        self.assertEqual(pc.color_bgr555(255, 0, 0), bytes((0x1F, 0x00)))
        self.assertEqual(pc.color_bgr555(0, 0, 255), bytes((0x00, 0x7C)))


class PruebaPiezas(unittest.TestCase):
    def cuadro(self, puntos, ancho=64, alto=32):
        img = Image.new("P", (ancho, alto), 0)
        for x, y in puntos:
            img.putpixel((x, y), 3)
        return img

    def test_solo_las_piezas_con_pixeles_y_en_orden(self):
        img = self.cuadro([(40, 3), (1, 20), (17, 2)])
        self.assertEqual(pc.piezas_no_vacias(img), [(16, 0), (32, 0), (0, 16)])

    def test_vacio_no_tiene_piezas(self):
        self.assertEqual(pc.piezas_no_vacias(self.cuadro([])), [])

    def test_tile_de_pieza(self):
        self.assertEqual([pc.tile_de_pieza(k) for k in (0, 1, 7, 8, 9)], [0, 2, 14, 32, 34])

    def test_empaquetado_en_pares_de_filas(self):
        # 9 piezas: hacen falta 2 pares de filas (2 KB); la novena va al tile 32.
        puntos = [(x * 16, 0) for x in range(4)] + [(x * 16, 16) for x in range(4)] + [(0, 32)]
        img = self.cuadro(puntos, alto=48)
        piezas = pc.piezas_no_vacias(img)
        self.assertEqual(len(piezas), 9)
        datos = pc.empaquetar(img, piezas)
        self.assertEqual(len(datos), 2 * pc.BYTES_PAR)
        # La pieza 8 tiene su píxel (valor 3: planos 0 y 1) arriba a la izquierda:
        # tile 32, fila 0, bit 7.
        self.assertEqual(datos[32 * 32], 0x80)
        self.assertEqual(datos[32 * 32 + 1], 0x80)
        # La pieza 1 (tile 2): su píxel también arriba a la izquierda.
        self.assertEqual(datos[2 * 32], 0x80)


class PruebaRanuras(unittest.TestCase):
    def test_cuenta_por_linea(self):
        # Una pieza de 32 en y=10 (4 ranuras, líneas 10..41) y una de 16 en y=40 (2).
        ran, spr = pc.ranuras_por_linea([(10, [(0, 32)]), (40, [(0, 16)])])
        self.assertEqual(ran[9], 0)
        self.assertEqual(ran[10], 4)
        self.assertEqual(ran[40], 6)
        self.assertEqual(spr[41], 2)
        self.assertEqual(ran[42], 2)
        self.assertEqual(ran[56], 0)


if __name__ == "__main__":
    unittest.main()
