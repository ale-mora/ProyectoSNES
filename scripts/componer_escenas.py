#!/usr/bin/env python3
"""
componer_escenas.py — Compone TODOS los fondos de la demo a partir de las
descargas en res/descargas/ y los deja listos para gfx4snes en res/<pantalla>/.

Es determinista: si cambia un asset o una posición, se vuelve a ejecutar y
listo. Cada función documenta el formato que exige la pantalla que la usa.

Uso (desde la raíz del repo):  python scripts/componer_escenas.py

Salidas:
  res/nintendo/logo_nintendo.png     256x224, 16 colores  (Mode 1, BG1)
  res/publisher/logo_publisher.png   256x224, 16 colores  (Mode 1, BG1)
  res/devs/logo_devs.png             256x224, 16 colores  (Mode 1, BG1; logo propio del estudio)
  res/intro/escena_1.png             256x224, 256 colores (Mode 3; imagen arriba 256x120, banda transparente abajo)
  res/intro/escena_2.png             idem
  res/intro/escena_3.png             256x224, 16 colores  (Mode 1: convive con los sprites)
  res/titulo/fondo_lejos.png         256x224, 16 colores, cielo transparente (índice 0), logo horneado
  res/titulo/fondo_cerca.png         512x256, 16 colores, transparente arriba (bucle sin costura)
  res/titulo/cielo.bmp               256x224, degradado por línea para HDMA (gfx4snes -n)

Reglas SNES que aplica:
  - Índice 0 = transparente en capas superiores y color de fondo en la inferior.
  - En Mode 3 la fuente de texto usa la paleta 7 (colores 112..127): las
    escenas dejan esas 16 entradas libres.
"""

from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

D = Path("res/descargas/graficos")
FUENTES = Path("res/descargas/fuentes")
SDK_LOGOS = Path("tools/pvsneslib/snes-examples/thirdparty/logo")
ARK16 = FUENTES / "ark-pixel-font-16px-proportional-ttf-v2026.09.01/ark-pixel-16px-proportional-latin.ttf"
ARK10 = FUENTES / "ark-pixel-font-10px-proportional-ttf-v2026.09.01/ark-pixel-10px-proportional-latin.ttf"
SPRING = D / "Spring Forest 2D Pixel art"
AUTUMN = D / "Autumn Forest 2D Pixel Art/Autumn Forest 2D Pixel Art/Background"

ANCHO, ALTO = 256, 224
TRANSP = (255, 0, 255)          # magenta = índice 0 en las imágenes con transparencia
NEGRO = (0, 0, 0)


# ----------------------------------------------------------------------------- utilidades
def abrir(p: Path) -> Image.Image:
    return Image.open(p).convert("RGBA")


def escalar(im: Image.Image, factor: float) -> Image.Image:
    """Escala con vecino más cercano (conserva el pixel art)."""
    return im.resize((max(1, round(im.width * factor)), max(1, round(im.height * factor))), Image.Resampling.NEAREST)


def ajustar_ancho(im: Image.Image, ancho: int) -> Image.Image:
    return escalar(im, ancho / im.width)


def texto(im: Image.Image, xy, cadena, fuente, relleno=(255, 255, 255), borde=None, escala=1, centrado=True):
    """Dibuja texto pixelado (opcionalmente ampliado x2/x3 con vecino más cercano y con borde)."""
    f = ImageFont.truetype(str(fuente), fuente_tamano(fuente))
    bb = f.getbbox(cadena)
    w, h = bb[2] - bb[0], bb[3] - bb[1]
    capa = Image.new("RGBA", (w + 4, h + 4), (0, 0, 0, 0))
    d = ImageDraw.Draw(capa)
    if borde:
        for dx in (-1, 0, 1):
            for dy in (-1, 0, 1):
                d.text((2 - bb[0] + dx, 2 - bb[1] + dy), cadena, font=f, fill=borde)
    d.text((2 - bb[0], 2 - bb[1]), cadena, font=f, fill=relleno)
    # Quitar antialias: todo píxel con alfa > 0 pasa a opaco.
    capa = Image.merge("RGBA", capa.split()[:3] + (capa.getchannel("A").point(lambda a: 255 if a > 0 else 0),))
    if escala != 1:
        capa = escalar(capa, escala)
    x, y = xy
    if centrado:
        x = x - capa.width // 2
    im.paste(capa, (x, y), capa)
    return capa.size


def fuente_tamano(p: Path) -> int:
    return 16 if "16px" in p.name else 10


def indexar(im: Image.Image, colores: int, transparente: bool, reservar_112_127=False) -> Image.Image:
    """RGBA → imagen 'P'. Si `transparente`, el magenta/alfa 0 va al índice 0.
    Si `reservar_112_127`, los índices 112..127 quedan sin usar (paleta 7 para el texto)."""
    fondo = TRANSP if transparente else NEGRO
    plano = Image.new("RGB", im.size, fondo)
    plano.paste(im, (0, 0), im)
    n = colores - 1 if transparente else colores
    if reservar_112_127:
        n -= 16
    cuant = plano.quantize(colors=n, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    pal = cuant.getpalette()[: n * 3]
    src = cuant.load()
    out = Image.new("P", im.size, 0)
    dst = out.load()
    rgb = plano.load()
    # Construimos la paleta final: [transparente] + colores, saltando 112..127 si hace falta.
    paleta_final = [0] * 768
    mapa_idx = {}
    siguiente = 1 if transparente else 0
    for i in range(n):
        if reservar_112_127 and 112 <= siguiente <= 127:
            siguiente = 128
        mapa_idx[i] = siguiente
        paleta_final[siguiente * 3: siguiente * 3 + 3] = pal[i * 3: i * 3 + 3]
        siguiente += 1
    if transparente:
        # El índice 0 es transparente por su NÚMERO, no por su color. Lo dejamos
        # NEGRO porque en la capa inferior (Mode 3) el color 0 es el fondo de
        # toda la pantalla y se vería el magenta.
        paleta_final[0:3] = [0, 0, 0]
    out.putpalette(paleta_final)
    for y in range(im.height):
        for x in range(im.width):
            if transparente and rgb[x, y] == TRANSP:
                dst[x, y] = 0
            else:
                dst[x, y] = mapa_idx[src[x, y]]
    return out


def guardar(im: Image.Image, destino: Path):
    destino.parent.mkdir(parents=True, exist_ok=True)
    im.save(destino)
    usados = len(im.getcolors(256) or [])
    print(f"  {destino}  {im.size[0]}x{im.size[1]}  colores usados={usados}")


def tiles_unicos(im: Image.Image) -> int:
    """Cuenta tiles de 8x8 distintos (estimación de lo que hará gfx4snes)."""
    vistos = set()
    for ty in range(0, im.height, 8):
        for tx in range(0, im.width, 8):
            vistos.add(im.crop((tx, ty, tx + 8, ty + 8)).tobytes())
    return len(vistos)


# ----------------------------------------------------------------------------- capas base
def spring_compuesto(capas=(1, 2, 3, 4)) -> Image.Image:
    base = Image.new("RGBA", (384, 216), (0, 0, 0, 0))
    for n in capas:
        capa = abrir(SPRING / f"Background/layer_{n}.png")
        base.paste(capa, (0, 0), capa)
    return base


def autumn_compuesto() -> Image.Image:
    base = Image.new("RGBA", (320, 180), (0, 0, 0, 0))
    for n in (3, 2, 1):                          # 3 = fondo, 1 = frente
        capa = abrir(AUTUMN / f"{n}.png")
        base.paste(capa, (0, 0), capa)
    return base


def recorte(im: Image.Image, filas: int) -> Image.Image:
    """Escala a 256 de ancho y recorta `filas` filas centradas en el horizonte.
    Mode 3 (256 colores, 64 bytes por tile): 120 filas = 480 tiles como máximo,
    para que el .pic (más el tile vacío) quepa en un banco de 32 KB.
    Mode 1 (16 colores): 128 filas = 512 tiles, el máximo de 16 KB de VRAM."""
    im = ajustar_ancho(im, 256)
    y0 = max(0, (im.height - filas) // 2 + 6)
    return im.crop((0, y0, 256, y0 + filas))


def con_banda_transparente(imagen: Image.Image) -> Image.Image:
    """Imagen arriba y banda TRANSPARENTE (índice 0) abajo: por ahí se ve el
    texto de la capa de atrás y el color de fondo negro."""
    lienzo = Image.new("RGBA", (ANCHO, ALTO), (0, 0, 0, 0))
    lienzo.paste(imagen, (0, 0), imagen)
    return lienzo


# ----------------------------------------------------------------------------- pantallas
def nintendo():
    print("[nintendo]")
    lienzo = Image.new("RGBA", (ANCHO, ALTO), NEGRO + (255,))
    logo = ajustar_ancho(abrir(D / "nintendo_logo_960.png"), 176)   # 176x59
    lienzo.paste(logo, ((ANCHO - logo.width) // 2, 64), logo)
    guardar(indexar(lienzo, 16, transparente=False), Path("res/nintendo/logo_nintendo.png"))


def publisher():
    print("[publisher]")
    lienzo = Image.new("RGBA", (ANCHO, ALTO), NEGRO + (255,))
    logo = escalar(abrir(SDK_LOGOS / "snes-logo-capcom/res/logo.bmp"), 2)     # 192x64
    lienzo.paste(logo, ((ANCHO - logo.width) // 2, 80), logo)
    guardar(indexar(lienzo, 16, transparente=False), Path("res/publisher/logo_publisher.png"))


def devs():
    """Logo del estudio, propio: "DON POLLO" grande en dorado con borde oscuro,
    "STUDIOS" debajo y una barra de época. 256x224, 16 colores, Mode 1.
    (El companyLogo.bmp de Konami del SDK es un juego de tiles desordenado que
    su ejemplo recompone con un mapa propio; no sirve como imagen directa.)"""
    print("[devs]")
    lienzo = Image.new("RGBA", (ANCHO, ALTO), NEGRO + (255,))
    d = ImageDraw.Draw(lienzo)
    texto(lienzo, (128, 40), "DON POLLO", ARK16, (255, 214, 64), borde=(120, 60, 0), escala=2)
    texto(lienzo, (128, 84), "STUDIOS", ARK16, (255, 240, 180), borde=(120, 60, 0))
    # Barra dorada con sombra, estilo logo de los 90.
    d.rectangle([40, 110, 215, 115], fill=(255, 214, 64))
    d.rectangle([40, 116, 215, 119], fill=(160, 96, 0))
    guardar(indexar(lienzo, 16, transparente=False), Path("res/devs/logo_devs.png"))


def escenas_intro():
    print("[intro]")
    # Escena 1, el pueblo: todas las capas + herrería + torii + un cerezo.
    s1 = spring_compuesto()
    herreria = abrir(SPRING / "Props/Blacksmith.png").crop((4, 0, 171, 112))
    torii = abrir(SPRING / "Props/Torii gate.png")
    cerezo = abrir(SPRING / "Trees/Color 1.png").crop((0, 0, 150, 160))
    suelo = 204
    s1.paste(cerezo, (0, suelo - cerezo.height), cerezo)
    s1.paste(herreria, (120, suelo - herreria.height), herreria)
    s1.paste(torii, (300, suelo - torii.height), torii)
    guardar(indexar(con_banda_transparente(recorte(s1, 120)), 256, transparente=True, reservar_112_127=True),
            Path("res/intro/escena_1.png"))
    # Escena 2, el viento: bosque otoñal oscuro.
    guardar(indexar(con_banda_transparente(recorte(autumn_compuesto(), 120)), 256, transparente=True, reservar_112_127=True),
            Path("res/intro/escena_2.png"))
    # Escena 3, el camino: cielo, montañas y pinos con el torii; 16 colores (Mode 1).
    s3 = spring_compuesto((1, 2, 3))
    s3.paste(torii, (144, 214 - torii.height), torii)
    im3 = indexar(con_banda_transparente(recorte(s3, 128)), 16, transparente=True)
    print("  escena 3: tiles distintos", tiles_unicos(im3))
    guardar(im3, Path("res/intro/escena_3.png"))


def titulo():
    print("[titulo]")
    # Capa lejana: montañas + pinos abajo, cielo transparente, logo horneado arriba.
    lejos = Image.new("RGBA", (ANCHO, ALTO), (0, 0, 0, 0))
    montes = ajustar_ancho(spring_compuesto((2, 3)), 256)              # 256x144, transparente arriba
    lejos.paste(montes, (0, ALTO - montes.height), montes)
    texto(lejos, (128, 20), "TAROUH", ARK16, (255, 255, 255), borde=(40, 20, 60), escala=2)
    texto(lejos, (128, 62), "A DOG'S JOURNEY", ARK16, (255, 220, 240), borde=(40, 20, 60))
    im = indexar(lejos, 16, transparente=True)
    print("  lejos: tiles distintos", tiles_unicos(im))
    guardar(im, Path("res/titulo/fondo_lejos.png"))
    # Capa cercana: arbustos (capa 4) en bucle de 512 px (original + espejo), transparente arriba.
    # 512x256 (no 224): gfx4snes -y reparte el mapa en páginas de 32x32 tiles y
    # necesita un alto múltiplo de 32. Las 32 filas de abajo quedan fuera de la
    # pantalla (que mide 224) y transparentes.
    cerca = Image.new("RGBA", (512, 256), (0, 0, 0, 0))
    arbustos = ajustar_ancho(abrir(SPRING / "Background/layer_4.png"), 256)
    cerca.paste(arbustos, (0, ALTO - arbustos.height), arbustos)
    espejo = arbustos.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
    cerca.paste(espejo, (256, ALTO - espejo.height), espejo)
    im = indexar(cerca, 16, transparente=True)
    print("  cerca: tiles distintos", tiles_unicos(im))
    guardar(im, Path("res/titulo/fondo_cerca.png"))
    # Cielo: un color por línea (gfx4snes -n lee la columna 0). Azul noche arriba → celeste al horizonte.
    cielo = Image.new("RGB", (ANCHO, ALTO))
    d = ImageDraw.Draw(cielo)
    arriba, horizonte = (16, 40, 120), (150, 220, 255)
    for y in range(ALTO):
        t = min(1.0, y / 150)
        col = tuple(round(arriba[i] + (horizonte[i] - arriba[i]) * t) for i in range(3))
        d.line([(0, y), (ANCHO - 1, y)], fill=col)
    cielo_p = cielo.quantize(colors=256, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    Path("res/titulo").mkdir(parents=True, exist_ok=True)
    cielo_p.save("res/titulo/cielo.bmp", format="BMP")
    print("  res/titulo/cielo.bmp 256x224 (degradado por línea)")


if __name__ == "__main__":
    nintendo()
    publisher()
    devs()
    escenas_intro()
    titulo()
