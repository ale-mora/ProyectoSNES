"""
prueba_preparar_nivel.py — Pruebas de scripts/preparar_nivel.py y de
scripts/mapa_a_columnas.py.

Se ejecutan desde la raíz del repositorio:
    python -m unittest scripts/pruebas/prueba_preparar_nivel.py

Comprueban: que el .tmj que escribe el script se lee igual (y sin espacios tras
los dos puntos, la trampa de tmx2snes), que se ignoran los bits de volteo de
Tiled, que una hoja de «rejilla doble» falsa da las 16 combinaciones de
esquinas, que la cuenta de tiles trata los volteados como iguales, que el
nivel completo sale con las medidas de docs/diseno-tutorial-movimiento.md y
que el mapa se reordena bien de filas a columnas.
"""

import json
import struct
import sys
import tempfile
import unittest
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import mapa_a_columnas as mc  # noqa: E402
import preparar_nivel as pn   # noqa: E402

T = 16
TIERRA = (140, 90, 30, 255)
HIERBA = (60, 200, 60, 255)


def hoja_doble(ruta: Path, sin_diagonales: bool = True) -> None:
    """Hoja de 16 celdas de rejilla doble: la celda k tiene tierra en los cuartos
    cuyas esquinas marca k (TL=1, TR=2, BL=4, BR=8), con una franja de hierba en lo
    alto de cada cuarto de abajo cuando el de arriba está vacío."""
    im = Image.new("RGBA", (16 * T, T), (0, 0, 0, 0))
    for k in range(16):
        if sin_diagonales and k in (6, 9):
            continue
        for b, (x0, y0) in enumerate([(0, 0), (8, 0), (0, 8), (8, 8)]):
            if k >> b & 1:
                for y in range(8):
                    for x in range(8):
                        im.putpixel((k * T + x0 + x, y0 + y), TIERRA if y > 1 else HIERBA)
    im.save(ruta)


class PruebaTmj(unittest.TestCase):
    def test_ida_y_vuelta(self):
        g, objetos = pn.trazado_inicial()
        with tempfile.TemporaryDirectory() as d:
            ruta = Path(d) / "n.tmj"
            pn.escribir_tmj(ruta, g, objetos)
            texto = ruta.read_text(encoding="utf-8")
            self.assertNotIn('": ', texto)            # sin espacio tras los dos puntos
            g2, objetos2 = pn.leer_tmj(ruta)
        self.assertEqual(g, g2)
        self.assertEqual([o["nombre"] for o in objetos], [o["nombre"] for o in objetos2])
        self.assertEqual(objetos[1]["texto"], objetos2[1]["texto"])

    def test_bits_de_volteo(self):
        g, objetos = pn.trazado_inicial()
        with tempfile.TemporaryDirectory() as d:
            ruta = Path(d) / "n.tmj"
            pn.escribir_tmj(ruta, g, objetos)
            mapa = json.loads(ruta.read_text(encoding="utf-8"))
            datos = mapa["layers"][0]["data"]
            datos[0] = pn.SOLIDO | 0x80000000          # el autor volteó un tile en Tiled
            ruta.write_text(json.dumps(mapa, separators=(",", ":")), encoding="utf-8")
            g2, _ = pn.leer_tmj(ruta)
        self.assertEqual(g2[0][0], pn.SOLIDO)

    def test_tamano_equivocado(self):
        with tempfile.TemporaryDirectory() as d:
            ruta = Path(d) / "n.tmj"
            ruta.write_text(json.dumps({"width": 64, "height": 14, "tilewidth": 16, "layers": []}))
            with self.assertRaises(SystemExit):
                pn.leer_tmj(ruta)


class PruebaTrazado(unittest.TestCase):
    """Las medidas del recorrido (diseno-tutorial-movimiento §2) en la rejilla inicial."""

    def setUp(self):
        self.g, self.objetos = pn.trazado_inicial()

    def alto_en(self, col):                            # px de lo sólido sobre el suelo base
        return 176 - pn.suelo_bajo(self.g, col)

    def test_obstaculos(self):
        self.assertEqual(self.alto_en(22), 16)         # escalón
        self.assertEqual(self.alto_en(25), 32)         # escalón alto
        self.assertEqual(self.alto_en(38), 48)         # muro
        self.assertEqual(pn.suelo_bajo(self.g, 72), pn.VISIBLE)   # foso: nada debajo
        foso = [c for c in range(pn.COLS) if self.g[11][c] == pn.VACIO]
        self.assertEqual(len(foso) * T, 96)
        techo = max(f for f in range(pn.FILAS) if self.g[f][55] == pn.SOLIDO and f < 11)
        self.assertEqual(176 - (techo + 1) * T, 48)    # hueco del túnel
        self.assertIn(pn.ZARZA, self.g[10])

    def test_objetos(self):
        nombres = [o["nombre"] for o in self.objetos]
        self.assertEqual(nombres.count("cartel"), 8)
        for n in ("inicio", "tronco", "salida"):
            self.assertEqual(nombres.count(n), 1)


class PruebaArte(unittest.TestCase):
    def test_esquinas_y_diagonales(self):
        with tempfile.TemporaryDirectory() as d:
            ruta = Path(d) / "h.png"
            hoja_doble(ruta)
            piezas = pn.piezas_por_esquinas(Image.open(ruta).convert("RGBA"))
        self.assertEqual(sorted(piezas), list(range(16)))   # 6 y 9 sustituidas

    def test_tiles_volteados_cuentan_uno(self):
        im = Image.new("P", (16, 8), 0)
        im.putpixel((0, 0), 1)                         # tile 1: píxel arriba a la izquierda
        im.putpixel((15, 0), 1)                        # tile 2: el mismo, volteado
        self.assertEqual(pn.tiles_distintos(im), 1)
        im.putpixel((15, 7), 1)
        self.assertEqual(pn.tiles_distintos(im), 2)

    def test_nivel_completo(self):
        g, objetos = pn.trazado_inicial()
        with tempfile.TemporaryDirectory() as d:
            d = Path(d)
            pn.escribir_tmj(d / "n.tmj", g, objetos)
            hoja_doble(d / "h.png")
            n = pn.preparar(d / "n.tmj", d / "h.png", d / "c.png", d / "c.bin", d / "c.h")
            im = Image.open(d / "c.png")
            self.assertEqual((im.size, im.mode), ((2048, 256), "P"))
            self.assertLessEqual(max(im.tobytes()), 15)       # 15 colores + transparente
            self.assertEqual((d / "c.bin").read_bytes(), bytes(v for f in g for v in f))
            cab = (d / "c.h").read_text(encoding="utf-8")
            self.assertIn("#define NIVEL1_CONTROLES 8", cab)
            self.assertIn("#define NIVEL1_INICIO_X 40", cab)
        self.assertLessEqual(n, pn.LIMITE_TILES)

    def test_limite_de_tiles(self):
        g, objetos = pn.trazado_inicial()
        with tempfile.TemporaryDirectory() as d:
            d = Path(d)
            pn.escribir_tmj(d / "n.tmj", g, objetos)
            hoja_doble(d / "h.png")
            with self.assertRaises(SystemExit):
                pn.preparar(d / "n.tmj", d / "h.png", d / "c.png", d / "c.bin", d / "c.h", limite=10)
            self.assertFalse((d / "c.png").exists())           # no escribe nada


class PruebaColumnas(unittest.TestCase):
    def test_reordenar(self):
        ancho, alto = 4, 3
        filas = struct.pack(f"<{ancho * alto}H", *range(ancho * alto))   # (x, y) = y*4 + x
        col = struct.unpack(f"<{ancho * alto}H", mc.a_columnas(filas, ancho))
        self.assertEqual(col[:3], (0, 4, 8))           # columna 0, de arriba abajo
        self.assertEqual(col[3:6], (1, 5, 9))          # columna 1

    def test_ancho_que_no_cuadra(self):
        with self.assertRaises(ValueError):
            mc.a_columnas(bytes(10), 4)


if __name__ == "__main__":
    unittest.main()
