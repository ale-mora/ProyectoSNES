"""
prueba_preparar_suelo.py — Pruebas de scripts/preparar_suelo.py.

Se ejecutan desde la raíz del repositorio:
    python -m unittest scripts/pruebas/prueba_preparar_suelo.py

Cada prueba fabrica una hoja de tiles falsa con la forma de la exportación
«Sidescroller» de Pixellab (tiles de 16x16: unos de BORDE, con las filas de
arriba vacías y hierba encima de tierra, y otros de TIERRA, opacos), llama a
preparar() y comprueba la capa cercana resultante: 512x256 indexada, la hierba
empezando justo en la fila del suelo, tierra opaca hasta abajo, a lo sumo 15
colores + transparente, y la tierra variada con volteos para que no se vean
franjas. Los valores son los de docs/diseno-demo-jugable.md §8.
"""

import sys
import tempfile
import unittest
from pathlib import Path

from PIL import Image, ImageDraw

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import preparar_suelo as ps  # noqa: E402

T = 16
VERDE = (60, 200, 60, 255)
MARRON = (140, 90, 30, 255)
PIEDRA = (150, 150, 150, 255)
OSCURO = (70, 40, 10, 255)


def hoja_falsa(ruta: Path, con_borde: bool = True, matas: bool = False) -> None:
    """Hoja de 8x4 tiles: BORDE en la fila 3 (columnas 0..3) y TIERRA en la fila 0
    (columnas 6 y 7, con una piedra en la esquina para que los volteos se noten).
    El resto, transparente o con formas que no son ni borde ni tierra. Con
    matas=True, algunas columnas del borde tienen briznas sueltas en las filas 5..7
    del tile, por encima de la banda llena (como la hoja de Pixellab con hierba
    gruesa)."""
    im = Image.new("RGBA", (8 * T, 4 * T), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    if con_borde:
        for c in range(4):
            x = c * T
            d.rectangle((x, 3 * T + 8, x + T - 1, 3 * T + 9), fill=VERDE)       # hierba: filas 8..9
            d.rectangle((x, 3 * T + 10, x + T - 1, 4 * T - 1), fill=MARRON)     # tierra: filas 10..15
            if matas:
                d.rectangle((x + 3, 3 * T + 5, x + 3, 3 * T + 7), fill=VERDE)    # brizna: filas 5..7
    for c in (6, 7):
        x = c * T
        d.rectangle((x, 0, x + T - 1, T - 1), fill=MARRON)
        d.rectangle((x, 0, x + 2, 2), fill=PIEDRA)                               # marca asimétrica
    d.rectangle((2 * T, T, 3 * T - 1, 2 * T - 4), fill=OSCURO)                  # ni borde ni tierra
    im.save(ruta)


def seto_falso(ruta: Path) -> None:
    """Arbustos como los del título: 512x256 indexada, 3 colores, filas 177..223."""
    im = Image.new("P", (512, 256), 0)
    im.putpalette([0, 0, 0, 78, 160, 97, 61, 127, 101, 52, 98, 98] + [0] * (256 * 3 - 12))
    d = ImageDraw.Draw(im)
    for x in range(0, 512, 24):
        d.ellipse((x, 177, x + 30, 223), fill=1 + (x // 24) % 3)
    im.save(ruta)


class PruebasPrepararSuelo(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.dir = Path(self.tmp.name)
        self.hoja = self.dir / "hoja.png"
        self.salida = self.dir / "fondo_cerca.png"
        hoja_falsa(self.hoja)

    def tearDown(self):
        self.tmp.cleanup()

    def test_tamano_paleta_y_transparente(self):
        """512x256 indexada (gfx4snes -y exige alto múltiplo de 256), índice 0 =
        transparente arriba, como mucho 16 índices en uso."""
        ps.preparar(self.hoja, self.salida, suelo_y=176)
        with Image.open(self.salida) as im:
            self.assertEqual(im.size, (512, 256))
            self.assertEqual(im.mode, "P")
            usados = set(im.get_flattened_data() if hasattr(im, "get_flattened_data") else im.getdata())
            self.assertLessEqual(len(usados), 16)
            self.assertEqual(im.getpixel((0, 0)), 0)

    def test_hierba_en_la_fila_del_suelo(self):
        """En cada columna, el primer píxel opaco está en la fila del suelo, y de ahí
        hasta la última fila visible (223) todo es opaco: no hay huecos en el piso."""
        ps.preparar(self.hoja, self.salida, suelo_y=176)
        with Image.open(self.salida) as im:
            px = im.load()
            for x in range(0, 512, 7):
                col = [px[x, y] for y in range(224)]
                self.assertEqual(next(y for y, v in enumerate(col) if v), 176, f"columna {x}")
                self.assertTrue(all(col[176:224]), f"hueco en la columna {x}")

    def test_otra_altura(self):
        """La altura del suelo es un parámetro (SUELO_Y del juego)."""
        ps.preparar(self.hoja, self.salida, suelo_y=192)
        with Image.open(self.salida) as im:
            col = [im.getpixel((100, y)) for y in range(224)]
            self.assertEqual(next(y for y, v in enumerate(col) if v), 192)

    def test_tierra_variada_con_volteos(self):
        """Las celdas de tierra no son todas iguales: se voltean (gratis en el mapa de
        la SNES) para que la textura no forme franjas regulares; y cada celda es una
        de las de la hoja, volteada o no (no se inventa arte)."""
        ps.preparar(self.hoja, self.salida, suelo_y=176)
        with Image.open(self.salida) as im:
            rgb = im.convert("RGB")
        # La hierba de la hoja falsa empieza en la fila 8 del tile de borde: el borde va
        # de 168 a 183 y la primera fila de celdas de tierra empieza en la 184.
        celdas = {rgb.crop((x, 184, x + T, 184 + T)).tobytes() for x in range(0, 512, T)}
        self.assertGreater(len(celdas), 1)
        with Image.open(self.hoja) as h:
            fuente = h.convert("RGB")
        validas = set()
        for c in (6, 7):
            t = fuente.crop((c * T, 0, c * T + T, T))
            for v in (t, t.transpose(Image.FLIP_LEFT_RIGHT), t.transpose(Image.FLIP_TOP_BOTTOM),
                      t.transpose(Image.ROTATE_180)):
                validas.add(v.tobytes())
        self.assertTrue(celdas <= validas)

    def test_determinista(self):
        """Dos ejecuciones dan el mismo archivo: la variación usa una semilla fija."""
        ps.preparar(self.hoja, self.salida, suelo_y=176)
        a = self.salida.read_bytes()
        ps.preparar(self.hoja, self.salida, suelo_y=176)
        self.assertEqual(a, self.salida.read_bytes())

    def test_matas_sobre_la_banda(self):
        """Las briznas sueltas que asoman por encima de la hierba no cuentan: lo que
        cae en la fila del suelo es la primera fila LLENA de la banda."""
        hoja_falsa(self.hoja, matas=True)
        ps.preparar(self.hoja, self.salida, suelo_y=176)
        with Image.open(self.salida) as im:
            px = im.load()
            primera_llena = next(y for y in range(224) if all(px[x, y] for x in range(512)))
            self.assertEqual(primera_llena, 176)
            self.assertTrue(any(px[x, 173] for x in range(512)))     # las briznas, por encima

    def test_hundir(self):
        """Con hundir=4 la banda de hierba empieza 4 px por encima de la fila del
        suelo: los pies del protagonista (que acaban en suelo_y - 1) quedan dentro
        de la hierba y no posados sobre su borde. Hace falta porque el arte de
        Pixellab es una vista lateral un poco elevada."""
        ps.preparar(self.hoja, self.salida, suelo_y=176, hundir=4)
        with Image.open(self.salida) as im:
            px = im.load()
            primera_llena = next(y for y in range(224) if all(px[x, y] for x in range(512)))
            self.assertEqual(primera_llena, 172)

    def test_seto_detras_del_suelo(self):
        """Con --seto, los arbustos asoman por ENCIMA del suelo (subidos para que su
        base quede tapada por la tierra) y el total sigue en 15 colores + transparente."""
        seto = self.dir / "seto.png"
        seto_falso(seto)
        ps.preparar(self.hoja, self.salida, suelo_y=176, seto=seto)
        with Image.open(self.salida) as im:
            px = im.load()
            arriba = sum(1 for y in range(140, 176) for x in range(512) if px[x, y])
            self.assertGreater(arriba, 0)
            usados = set(im.get_flattened_data() if hasattr(im, "get_flattened_data") else im.getdata())
            self.assertLessEqual(len(usados), 16)
            col = [px[200, y] for y in range(224)]
            self.assertTrue(all(col[176:224]))

    def test_error_sin_borde(self):
        """Una hoja sin tiles de borde (hierba sobre tierra) no sirve: el script falla
        sin escribir nada."""
        hoja_falsa(self.hoja, con_borde=False)
        with self.assertRaises(SystemExit):
            ps.preparar(self.hoja, self.salida, suelo_y=176)
        self.assertFalse(self.salida.exists())


if __name__ == "__main__":
    unittest.main()
