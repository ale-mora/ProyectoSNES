"""
prueba_preparar_personaje.py — Pruebas de scripts/preparar_personaje.py.

Se ejecutan desde la raíz del repositorio:
    python -m unittest scripts/pruebas/prueba_preparar_personaje.py

Cada prueba crea una carpeta temporal de entrada (lo que el autor descargaría de
Pixellab) y otra de salida (lo que iría a res/personajes/<nombre>/), llama a
preparar() y comprueba los archivos resultantes. Los valores (celda de 96x64,
10 cuadros como máximo, una paleta compartida, pies al borde inferior) son los de
docs/diseno-demo-jugable.md §7 y §10. Las entradas de los formatos sencillos son
cuadros cuadrados de 64x64; las de la exportación nativa de Pixellab imitan su
estructura (<Estado>/animations/<nombre>/<dirección>/frame_NNN.png).
"""

import sys
import tempfile
import unittest
from pathlib import Path

from PIL import Image, ImageDraw

# El script vive en scripts/; lo hacemos importable desde scripts/pruebas/.
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import preparar_personaje as pp  # noqa: E402

CELDA = 64          # lado de los cuadros de ENTRADA de las pruebas (formatos sencillos)
ANCHO = 96          # celda de SALIDA: 96x64 (3x2 piezas de 32x32)
ALTO = 64


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
    im = im.crop((cuadro * ANCHO, 0, (cuadro + 1) * ANCHO, ALTO))
    filas = [y for y in range(ALTO) for x in range(ANCHO) if im.getpixel((x, y)) != 0]
    return max(filas) if filas else -1


def caja_opaca(im: Image.Image, cuadro: int = 0):
    """Caja (x0, y0, x1, y1) de los píxeles no transparentes del cuadro indicado."""
    im = im.crop((cuadro * ANCHO, 0, (cuadro + 1) * ANCHO, ALTO))
    return im.point(lambda i: 255 if i != 0 else 0).getbbox()


# Once colores bien separados: el cuadro k de cada animación falsa se pinta con
# COLORES_CUADRO[k], así el orden se puede leer en la salida (11 < 15 colores).
COLORES_CUADRO = [(230, 30, 30), (30, 200, 30), (30, 30, 230), (230, 230, 30), (30, 220, 220),
                  (220, 30, 220), (250, 140, 20), (120, 60, 0), (250, 250, 250), (90, 90, 90), (150, 0, 90)]


def exportar_pixellab(raiz: Path, estado: str, direccion: str, cuadros: int, lienzo=(88, 88), ancho=60, alto=50):
    """Imita la exportación nativa de Pixellab: un rectángulo de ancho x alto centrado
    en el lienzo, con los pies 16 px por encima del borde inferior."""
    carpeta = raiz / estado / "animations" / f"{estado}_anim" / direccion
    carpeta.mkdir(parents=True)
    for k in range(cuadros):
        im = Image.new("RGBA", lienzo, (0, 0, 0, 0))
        x0 = (lienzo[0] - ancho) // 2
        y1 = lienzo[1] - 16
        ImageDraw.Draw(im).rectangle((x0, y1 - alto, x0 + ancho - 1, y1 - 1), fill=COLORES_CUADRO[k] + (255,))
        im.save(carpeta / f"frame_{k:03d}.png")


def rotacion_pixellab(raiz: Path, estado: str, direccion: str, ancho=64, alto=57):
    """Una pose fija (rotations/<dirección>.png) en un lienzo de 64x64."""
    carpeta = raiz / estado / "rotations"
    carpeta.mkdir(parents=True)
    im = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    ImageDraw.Draw(im).rectangle(((64 - ancho) // 2, 60 - alto, (64 - ancho) // 2 + ancho - 1, 59), fill=COLORES_CUADRO[0] + (255,))
    im.save(carpeta / f"{direccion}.png")


def orden_de(png: Path):
    """Qué cuadro de la animación de origen hay en cada cuadro de la salida, por el
    color más cercano del píxel central de su caja opaca."""
    with Image.open(png) as im:
        rgb = im.convert("RGB")
        n = im.width // ANCHO
        res = []
        for k in range(n):
            b = caja_opaca(im, k)
            c = rgb.getpixel((k * ANCHO + (b[0] + b[2]) // 2, (b[1] + b[3]) // 2))
            res.append(min(range(len(COLORES_CUADRO)), key=lambda j: sum((a - q) ** 2 for a, q in zip(c, COLORES_CUADRO[j]))))
        return res


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
        self.assertEqual(len(pngs), 14)          # 13 animaciones + tarou_reposo_estatico.png
        for png in pngs:
            with Image.open(png) as im:
                self.assertEqual(im.height, ALTO, png.name)
                self.assertEqual(im.width % ANCHO, 0, png.name)
                self.assertEqual(im.mode, "P", png.name)
        header = self.salida / "tarou_anim.h"
        self.assertEqual(cuadros_del_header(header, "reposo"), 4)
        self.assertEqual(cuadros_del_header(header, "caminar"), 8)
        self.assertEqual(cuadros_del_header(header, "atacar"), 5)
        self.assertEqual(resultado["reposo"], 4)
        self.assertIn("#define TAROU_ANIMACIONES 13", header.read_text(encoding="utf-8"))

    def test_reposo_estatico_siempre_dos_cuadros(self):
        """La tira estática de reposo (intro y título, personajes.c) tiene SIEMPRE
        2 cuadros de 96x64 (más no caben en los 512 tiles de sprites junto a
        Bárbara), repartidos a lo largo del reposo real: con 6 cuadros toma el 0
        y el 3; con 2, el 0 y el 1."""
        colores = [(200, 0, 0), (0, 200, 0), (0, 0, 200), (200, 200, 0), (0, 200, 200), (120, 60, 0)]

        def reposo_de(n):
            im = Image.new("RGBA", (CELDA * n, CELDA), (0, 0, 0, 0))
            d = ImageDraw.Draw(im)
            for k in range(n):
                d.rectangle((k * CELDA + 20, 10, k * CELDA + 43, 59), fill=colores[k] + (255,))
            return im

        def cuadros_de(png):
            """Qué cuadro del reposo hay en cada uno de los 4 de la tira estática,
            por el color MÁS CERCANO (la paleta común se reduce a 15 colores y
            puede mover un canal unas unidades)."""
            with Image.open(png) as im:
                self.assertEqual(im.size, (ANCHO * 2, ALTO))
                rgb = im.convert("RGB")
                vistos = [rgb.getpixel((k * ANCHO + 48, 40)) for k in range(2)]
            def cercano(c):
                return min(range(len(colores)), key=lambda j: sum((a - b) ** 2 for a, b in zip(c, colores[j])))
            return [cercano(c) for c in vistos]

        reposo_de(6).save(self.entrada / "reposo.png")
        pp.preparar("tarou", self.entrada, self.salida)
        self.assertEqual(cuadros_de(self.salida / "tarou_reposo_estatico.png"), [0, 3])
        # El reposo del nivel sigue entero (6 cuadros) y el header no cuenta la estática.
        self.assertEqual(cuadros_del_header(self.salida / "tarou_anim.h", "reposo"), 6)
        self.assertIn("#define TAROU_ANIMACIONES 13", (self.salida / "tarou_anim.h").read_text(encoding="utf-8"))

        reposo_de(2).save(self.entrada / "reposo.png")
        pp.preparar("tarou", self.entrada, self.salida)
        self.assertEqual(cuadros_de(self.salida / "tarou_reposo_estatico.png"), [0, 1])

    def test_tira_png(self):
        """Una tira PNG de 3 cuadros de 64x64 produce una salida de 288x64 (3 celdas
        de 96x64, sin escalar: el cuadro se centra) y CUADROS 3."""
        tira(3).save(self.entrada / "reposo.png")
        pp.preparar("tarou", self.entrada, self.salida)
        with Image.open(self.salida / "tarou_reposo.png") as im:
            self.assertEqual(im.size, (ANCHO * 3, ALTO))
            b = caja_opaca(im)
            self.assertEqual(b[2] - b[0], 24)       # el rectángulo de 24 px no se escaló
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

    def test_error_mas_de_10_cuadros(self):
        """Una tira de 11 cuadros (11 x 3 KB = 33 KB) no cabe en un banco de 32 KB:
        el script falla y no escribe nada."""
        tira(11).save(self.entrada / "caminar.png")
        with self.assertRaises(SystemExit) as ctx:
            pp.preparar("tarou", self.entrada, self.salida)
        self.assertNotEqual(ctx.exception.code, 0)
        self.assertFalse(self.salida.exists() and any(self.salida.iterdir()))


class PruebasPixellabNativo(unittest.TestCase):
    """La exportación nativa de Pixellab (Characters → Export: una carpeta por estado,
    8 direcciones, lienzos de 76..104 px) entra por el mapeo de preparar_personaje.py."""

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.entrada = Path(self.tmp.name) / "Tarou"
        self.salida = Path(self.tmp.name) / "salida"
        for estado, n in [("Idle", 9), ("Walk", 8), ("Unsheathe", 9), ("Jump", 9), ("Sneak", 8)]:
            exportar_pixellab(self.entrada, estado, "east", n)
        exportar_pixellab(self.entrada, "Run", "east", 8, lienzo=(96, 96), ancho=80, alto=46)
        exportar_pixellab(self.entrada, "Stand_up", "east", 11, lienzo=(104, 84))
        exportar_pixellab(self.entrada, "Sleep", "south", 9, lienzo=(76, 76), ancho=56, alto=40)
        rotacion_pixellab(self.entrada, "Attack_1", "east")
        self.resultado = pp.preparar("tarou", self.entrada, self.salida)

    def tearDown(self):
        self.tmp.cleanup()

    def test_cuadros_del_mapeo(self):
        """Cada animación del juego sale del estado de Pixellab que dice el mapeo."""
        esperado = {"reposo": 9, "caminar": 8, "correr": 8, "saltar": 3, "caer": 3, "agachado": 1,
                    "agachado_caminar": 8, "desenvainar": 9, "envainar": 9, "atacar": 1,
                    "echarse": 6, "dormido": 9, "despertar": 10}
        self.assertEqual(self.resultado, esperado)
        for animacion, n in esperado.items():
            self.assertEqual(cuadros_del_header(self.salida / "tarou_anim.h", animacion), n, animacion)

    def test_orden_de_los_cuadros(self):
        """Tramos e inversiones: saltar = Jump 2..4, caer = Jump 5..7, envainar =
        Unsheathe al revés, echarse = Stand_up 0..5 al revés, despertar = Stand_up
        0..9 (11 no caben en un banco), agachado = el primer cuadro de Sneak."""
        self.assertEqual(orden_de(self.salida / "tarou_saltar.png"), [2, 3, 4])
        self.assertEqual(orden_de(self.salida / "tarou_caer.png"), [5, 6, 7])
        self.assertEqual(orden_de(self.salida / "tarou_envainar.png"), [8, 7, 6, 5, 4, 3, 2, 1, 0])
        self.assertEqual(orden_de(self.salida / "tarou_echarse.png"), [5, 4, 3, 2, 1, 0])
        self.assertEqual(orden_de(self.salida / "tarou_despertar.png"), list(range(10)))
        self.assertEqual(orden_de(self.salida / "tarou_agachado.png"), [0])
        self.assertEqual(orden_de(self.salida / "tarou_dormido.png"), list(range(9)))   # mira al sur

    def test_sin_escalar_y_anclado(self):
        """Píxeles nativos: el perro de 80 px de Run sigue midiendo 80 px; cada
        animación queda centrada en la celda (centro del lienzo -> columna 48) y
        con los pies en la última fila."""
        with Image.open(self.salida / "tarou_correr.png") as im:
            b = caja_opaca(im)
            self.assertEqual((b[2] - b[0], b[3] - b[1]), (80, 46))
            self.assertEqual(b[3], ALTO)                   # pies en la fila 63
            self.assertEqual((b[0] + b[2]) // 2, ANCHO // 2)
        with Image.open(self.salida / "tarou_despertar.png") as im:    # lienzo de 104x84
            b = caja_opaca(im)
            self.assertEqual((b[0] + b[2]) // 2, ANCHO // 2)
            self.assertEqual(b[3], ALTO)
        with Image.open(self.salida / "tarou_atacar.png") as im:       # pose fija, lienzo 64x64
            self.assertEqual(im.size, (ANCHO, ALTO))
            b = caja_opaca(im)
            self.assertEqual(b[3], ALTO)
            self.assertEqual((b[0] + b[2]) // 2, ANCHO // 2)


class PruebasAtaqueNuevo(unittest.TestCase):
    """Unidad 19, ejercicio C: atacar sale de la animación «Attack» cuando la
    exportación la trae, y de la pose fija de Attack_1 mientras no."""

    def preparar_con(self, con_animacion: bool):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        entrada = Path(tmp.name) / "Tarou"
        exportar_pixellab(entrada, "Idle", "east", 9)
        rotacion_pixellab(entrada, "Attack_1", "east")
        if con_animacion:
            exportar_pixellab(entrada, "Attack", "east", 6)
        salida = Path(tmp.name) / "salida"
        return pp.preparar("tarou", entrada, salida), salida

    def test_sin_animacion_usa_la_pose(self):
        resultado, _ = self.preparar_con(False)
        self.assertEqual(resultado["atacar"], 1)

    def test_con_animacion_usa_la_animacion(self):
        resultado, salida = self.preparar_con(True)
        self.assertEqual(resultado["atacar"], 6)
        self.assertEqual(orden_de(salida / "tarou_atacar.png"), list(range(6)))


if __name__ == "__main__":
    unittest.main()
