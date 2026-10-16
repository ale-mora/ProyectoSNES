"""
prueba_preparar_personaje.py — Pruebas de scripts/preparar_personaje.py.

Se ejecutan desde la raíz del repositorio:
    python -m unittest scripts/pruebas/prueba_preparar_personaje.py

Cada prueba crea una carpeta temporal de entrada (lo que el autor descargaría de
Pixellab) y otra de salida (lo que iría a res/personajes/<nombre>/), llama a
preparar() y comprueba los archivos resultantes. Los valores (64x64, 15 cuadros
como máximo, una paleta compartida, pies al borde inferior) son los de
docs/diseno-demo-jugable.md §7 y §10.
"""

import sys
import tempfile
import unittest
from pathlib import Path

from PIL import Image, ImageDraw

# El script vive en scripts/; lo hacemos importable desde scripts/pruebas/.
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import preparar_personaje as pp  # noqa: E402

CELDA = 64


def tira(cuadros: int, caja=(20, 10, 44, 60), color=(200, 40, 40, 255)) -> Image.Image:
    """Tira RGBA de `cuadros` cuadros de 64x64 con un rectángulo de color en `caja`
    (x0, y0, x1, y1) en cada cuadro; el resto, transparente."""
    im = Image.new("RGBA", (CELDA * cuadros, CELDA), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for k in range(cuadros):
        x0, y0, x1, y1 = caja
        d.rectangle((k * CELDA + x0, y0, k * CELDA + x1 - 1, y1 - 1), fill=color)
    return im


def cuadros_del_header(header: Path, animacion: str, nombre: str = "tarou") -> int:
    texto = header.read_text(encoding="utf-8")
    clave = f"#define {nombre.upper()}_{animacion.upper()}_CUADROS "
    for linea in texto.splitlines():
        if linea.startswith(clave):
            return int(linea[len(clave):].strip())
    raise AssertionError(f"{clave!r} no está en {header}")


def ultima_fila_opaca(im: Image.Image, cuadro: int = 0) -> int:
    """Fila más baja con un píxel distinto del índice 0 en el cuadro indicado."""
    im = im.crop((cuadro * CELDA, 0, (cuadro + 1) * CELDA, CELDA))
    filas = [y for y in range(CELDA) for x in range(CELDA) if im.getpixel((x, y)) != 0]
    return max(filas) if filas else -1


class PruebasPrepararPersonaje(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.entrada = Path(self.tmp.name) / "pixellab"
        self.salida = Path(self.tmp.name) / "salida"
        self.entrada.mkdir()

    def tearDown(self):
        self.tmp.cleanup()

    def test_marcador_cuando_falta_animacion(self):
        """Sin entrada, salen 13 marcadores con los cuadros por defecto."""
        resultado = pp.preparar("tarou", self.entrada, self.salida)
        pngs = sorted(self.salida.glob("tarou_*.png"))
        self.assertEqual(len(pngs), 13)
        for png in pngs:
            with Image.open(png) as im:
                self.assertEqual(im.height, CELDA, png.name)
                self.assertEqual(im.mode, "P", png.name)
        header = self.salida / "tarou_anim.h"
        self.assertEqual(cuadros_del_header(header, "reposo"), 4)
        self.assertEqual(cuadros_del_header(header, "caminar"), 8)
        self.assertEqual(cuadros_del_header(header, "atacar"), 5)
        self.assertEqual(resultado["reposo"], 4)
        self.assertIn("#define TAROU_ANIMACIONES 13", header.read_text(encoding="utf-8"))

    def test_tira_png(self):
        """Una tira PNG de 3 cuadros produce una salida de 192x64 y CUADROS 3."""
        tira(3).save(self.entrada / "reposo.png")
        pp.preparar("tarou", self.entrada, self.salida)
        with Image.open(self.salida / "tarou_reposo.png") as im:
            self.assertEqual(im.size, (CELDA * 3, CELDA))
        self.assertEqual(cuadros_del_header(self.salida / "tarou_anim.h", "reposo"), 3)

    def test_carpeta_de_cuadros(self):
        """Una carpeta con 01.png y 02.png cuenta 2 cuadros."""
        carpeta = self.entrada / "caminar"
        carpeta.mkdir()
        tira(1).save(carpeta / "01.png")
        tira(1).save(carpeta / "02.png")
        pp.preparar("tarou", self.entrada, self.salida)
        self.assertEqual(cuadros_del_header(self.salida / "tarou_anim.h", "caminar"), 2)

    def test_gif(self):
        """Un GIF de 2 cuadros cuenta 2 cuadros."""
        cuadro_a = tira(1).convert("RGB")
        cuadro_b = tira(1, caja=(24, 10, 48, 60)).convert("RGB")
        cuadro_a.save(self.entrada / "correr.gif", save_all=True, append_images=[cuadro_b], duration=100, loop=0)
        pp.preparar("tarou", self.entrada, self.salida)
        self.assertEqual(cuadros_del_header(self.salida / "tarou_anim.h", "correr"), 2)

    def test_paleta_compartida(self):
        """Las 13 salidas comparten paleta, usan ≤ 16 índices y el fondo es el índice 0."""
        tira(2, color=(40, 200, 40, 255)).save(self.entrada / "reposo.png")
        tira(2, color=(40, 40, 200, 255)).save(self.entrada / "caminar.png")
        pp.preparar("tarou", self.entrada, self.salida)
        paletas = set()
        for png in self.salida.glob("tarou_*.png"):
            with Image.open(png) as im:
                paletas.add(tuple(im.getpalette()[:48]))
                self.assertLessEqual(len(set(im.get_flattened_data() if hasattr(im, "get_flattened_data") else im.getdata())), 16, png.name)
                self.assertEqual(im.getpixel((0, 0)), 0, png.name)
        self.assertEqual(len(paletas), 1, "todas las tiras deben llevar la misma paleta")

    def test_alineacion_comun(self):
        """Todas las animaciones se desplazan lo mismo: el personaje más bajo
        queda con los pies en la fila 63 y los demás conservan su altura relativa."""
        tira(1, caja=(20, 10, 44, 40)).save(self.entrada / "reposo.png")    # pies en la fila 39
        tira(1, caja=(20, 30, 44, 60)).save(self.entrada / "agachado.png")  # pies en la fila 59
        pp.preparar("tarou", self.entrada, self.salida)
        with Image.open(self.salida / "tarou_agachado.png") as im:
            self.assertEqual(ultima_fila_opaca(im), 63)
        with Image.open(self.salida / "tarou_reposo.png") as im:
            self.assertEqual(ultima_fila_opaca(im), 63 - 20)

    def test_error_mas_de_15_cuadros(self):
        """Una tira de 16 cuadros no cabe en un banco: el script falla y no escribe nada."""
        tira(16).save(self.entrada / "caminar.png")
        with self.assertRaises(SystemExit) as ctx:
            pp.preparar("tarou", self.entrada, self.salida)
        self.assertNotEqual(ctx.exception.code, 0)
        self.assertFalse(self.salida.exists() and any(self.salida.iterdir()))


if __name__ == "__main__":
    unittest.main()
