#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    preparar_aldea.py — La aldea al atardecer (zona 1 de la fase 1) para la SNES:
    capa cercana, capa lejana y degradado del cielo, a partir del arte de Pixellab.

    PROPÓSITO
      El arte llega en piezas sueltas (res/descargas/graficos/pixellab/aldea/,
      ignorado por git): dos fachadas machiya, un puesto con noren, la calle, un
      farol y unos barriles para el primer plano, cuatro aldeanos y el paisaje
      lejano. Este script:
        1. COMPONE la calle de 2048 px (las piezas en posiciones múltiplo de 8,
           para que al repetirse den los mismos tiles) en dos capas: el fondo y el
           primer plano (farol y barriles), que se dibuja POR DELANTE de los
           personajes con el bit de prioridad del mapa.
        2. CONVIERTE la cercana a tiles de 8x8 de 4bpp sin repetidos (también los
           volteados) con dos paletas: la de la arquitectura y la de los aldeanos;
           cada tile usa la que mejor lo dibuja. Si salen más tiles de los que caben
           en la VRAM, junta los más parecidos (y lo dice).
        3. RESERVA los tiles del noren (las 4x2 celdas de la cortina del puesto) al
           principio, sin compartir, y escribe su segundo cuadro aparte: el juego
           los cambia cada 8 frames (animación de tiles).
        4. Escribe el mapa COLUMNA A COLUMNA (como mapa_a_columnas.py), que es lo
           que fondo_nivel.c copia a la VRAM según avanza la cámara.
        5. La LEJANA: el paisaje (512 px, se repite) con un monte oscuro tapando la
           costura entre el borde derecho y el izquierdo, en un PNG indexado que el
           Makefile convierte con gfx4snes. Y el CIELO: un BMP con un color por
           línea, del violeta al naranja (gfx4snes -n, como el del título).
        6. El HUMO y los PÁJAROS (sprites de 16x16, 2 cuadros cada uno, paleta de
           sprites 7): sus tiles en el orden de la VRAM y, en aldea_datos.h, de qué
           tejados sale el humo (uno de cada dos tejados, para que nunca haya más
           de dos a la vista).

    SE CONECTA CON
      Lee:      res/descargas/graficos/pixellab/aldea/<pieza>/NN.png (FUENTES).
      Escribe:  res/aldea/cerca_til.bin, cerca_pal.bin, cerca_col.bin, noren_til.bin,
                ambiente_til.bin, ambiente_pal.bin, aldea_datos.h,
                cerca_vista.png (para mirarla), lejos.png, atardecer.bmp.
      Lo usan:  el Makefile (lejos.png, atardecer.bmp), data.asm (los .bin; con nombres .bin porque git ignora .pal y .col),
                src/juego/fondo_nivel.c y src/pantallas/pantalla_fase1.c (aldea_datos.h);
                scripts/pruebas/prueba_preparar_aldea.py.

    HARDWARE QUE TOCA
      ninguno; produce datos con el formato de la VRAM y la CGRAM:
        tile 4bpp   32 bytes (piezas16.tile_4bpp)
        mapa        palabra vhopppcc cccccccc: tile (10 bits), paleta (3), prioridad,
                    volteo horizontal y vertical
        paleta      16 colores BGR555 (el 0, transparente)

    VARIABLES: constantes de composición (abajo). Uso: python scripts/preparar_aldea.py
    → glosario: Tile, Tilemap, Paleta, Prioridad, Animación de tiles, Streaming de columnas
---------------------------------------------------------------------------------
"""
import struct
import sys
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
from piezas16 import tile_4bpp  # noqa: E402

RAIZ = Path(__file__).resolve().parents[1]
ORIGEN = RAIZ / "res/descargas/graficos/pixellab/aldea"
DESTINO = RAIZ / "res/aldea"

ANCHO = 2048                 # La fase entera (nivel.h: ANCHO_NIVEL).
ALTO = 256                   # 32 filas de tiles (el mapa de la VRAM es de 64x32).
COLUMNAS = ANCHO // 8
FILAS = ALTO // 8
LIMITE_TILES = 512           # VRAM_FONDO_A: 0x2000 palabras = 512 tiles de 32 bytes.

# Paletas (las mismas que se pidieron a Pixellab: las piezas ya vienen en ellas).
PALETA_ARQ = ["#160f1c", "#2e2238", "#4a3a56", "#6e5a78", "#4a2a22", "#7a4428", "#a8683a",
              "#d8a868", "#f0d8a0", "#8c6a50", "#5e4436", "#b88e62", "#e87830", "#c03828",
              "#f8e8c0"]
PALETA_GENTE = ["#160f1c", "#e8b08a", "#b47858", "#2a1e1e", "#2c3a6a", "#4e64a0", "#c03828",
                "#d89838", "#4e6a3a", "#8a8090", "#f0e8d8", "#7a4428", "#a8683a", "#d8a868",
                "#8c6a50"]
PALETA_LEJOS = ["#1e1428", "#2a1a3a", "#3e2a52", "#563c6a", "#745084", "#9a6a96", "#241830",
                "#3a2848", "#ffe0a0", "#f8a858", "#e8703a", "#f0a070", "#c06a6a", "#4a2e44",
                "#6a3e50"]
PALETA_OBJETOS = ["#160f1c", "#2e2238", "#4a3a56", "#786a80", "#988a98", "#b8aab0", "#d8ccc8",
                  "#f0e8e0", "#f8b868", "#e87830", "#c03828", "#f8e8c0", "#7a4428", "#a8683a",
                  "#4e6a3a"]       # Paleta de sprites 7: humo, pájaros (y los objetos que vengan).
PALETA_SNES_ARQ = 4          # Paletas de la CGRAM de la cercana (4 y 5: 64..95). La 0 y la
PALETA_SNES_GENTE = 5        # 1 son del texto (BG3 de 2bpp); la 2, de la lejana.

# Las piezas elegidas entre los candidatos de Pixellab (carpeta, número).
FUENTES = {
    "fachada_a": ("fachada", 0),
    "fachada_b": ("fachada", 3),
    "calle": ("calle", 8),
    "puesto": ("puesto", 0),
    "farol": ("farol", 3),
    "barriles": ("barriles", 6),
    "aldeano_1": ("aldeanos", 0),     # campesino con azada y sombrero de paja
    "aldeano_2": ("aldeanos", 1),     # mujer con kimono índigo
    "aldeano_3": ("aldeanos", 2),     # anciano con bastón
    "aldeano_4": ("aldeanos", 13),    # niña con kimono rojo y abanico
    "lejana": ("lejana", 0),
    "humo": ("humo", 3),              # 32x16: bocanada pequeña | penacho (2 cuadros)
    "pajaro": ("pajaro", 0),          # 32x16: alas arriba | alas abajo
}

SUELO_Y = 176                # Donde empieza la calle (nivel.h: SUELO_Y; el carril, 184..216).
FACHADA_Y = SUELO_Y - 105    # Las fachadas acaban (fila 104 de su imagen) justo encima.
PUESTO_Y = 112               # Múltiplo de 8: el noren cae en celdas fijas del puesto.
ALDEANO_Y = SUELO_Y + 2 - 56 # Los pies, sobre el borde de la calle.
NOREN = (16, 16, 32, 16)     # x, y, ancho, alto de la cortina dentro del puesto (4x2 tiles).
INTERIOR_PUESTO = (14, 20, 36, 25)   # Hueco bajo el tejado: se pinta oscuro (no se ve la lejana).
# La secuencia de la calle: pieza, volteada.
SECUENCIA = [("fachada_a", False), ("puesto", False), ("fachada_b", False),
             ("fachada_a", True), ("puesto", False), ("fachada_b", True)]
# Aldeanos: (aldeano, pieza de la secuencia a la que se arriman, x dentro de ella, volteado).
ALDEANOS = [("aldeano_1", 0, 40, False), ("aldeano_2", 2, 80, True),
            ("aldeano_3", 3, 72, False), ("aldeano_4", 5, 24, True)]
FAROLES = [196, 892, 1404, 1916]   # x del farol (16 px; el poste cae en una sola columna).
BARRILES = [520, 1176, 1712]       # x de la pila de barriles (32 px), abajo del todo.
HUMO_X = 88                  # Columna del tejado de donde sale el humo (en la fachada sin voltear).
LEJOS_Y = -40                # El paisaje sube 40 px: el castillo y el sol, sobre los tejados.


def hex_rgb(h):
    return tuple(int(h[i:i + 2], 16) for i in (1, 3, 5))


def bgr555(rgb):
    r, g, b = rgb
    return ((b >> 3) << 10) | ((g >> 3) << 5) | (r >> 3)


def paleta_bytes(colores):
    """Color 0 (transparente) + hasta 15 colores → 32 bytes BGR555."""
    vals = [0] + [bgr555(hex_rgb(c)) for c in colores]
    vals += [0] * (16 - len(vals))
    return struct.pack("<16H", *vals)


def cargar(nombre):
    carpeta, n = FUENTES[nombre]
    return Image.open(ORIGEN / carpeta / f"{n:02d}.png").convert("RGBA")


def pegar(lienzo, img, x, y, voltear=False):
    """alpha_composite con recorte (x, y pueden salirse del lienzo)."""
    if voltear:
        img = img.transpose(Image.FLIP_LEFT_RIGHT)
    w, h = img.size
    x0, y0 = max(0, -x), max(0, -y)
    x1, y1 = min(w, lienzo.width - x), min(h, lienzo.height - y)
    if x1 <= x0 or y1 <= y0:
        return
    lienzo.alpha_composite(img.crop((x0, y0, x1, y1)), (x + x0, y + y0))


def opacar(img):
    """Sin semitransparencias: alfa 0 o 255 (la SNES no tiene alfa)."""
    a = np.array(img)
    a[..., 3] = np.where(a[..., 3] >= 128, 255, 0)
    return Image.fromarray(a, "RGBA")


def preparar_puesto(puesto, color_interior):
    """El hueco bajo el tejado, oscuro, y los dos cuadros del noren (el 2.º, con las
    puntas de la cortina movidas un píxel: el viento)."""
    p = opacar(puesto)
    a = np.array(p)
    x, y, w, h = INTERIOR_PUESTO
    zona = a[y:y + h, x:x + w]
    vacio = zona[..., 3] == 0
    zona[vacio] = list(hex_rgb(color_interior)) + [255]
    cuadro1 = Image.fromarray(a.copy(), "RGBA")
    nx, ny, nw, nh = NOREN
    b = a.copy()
    for fila in range(ny + nh // 2, ny + nh):           # Mitad de abajo de la cortina.
        seg = b[fila, nx:nx + nw].copy()
        b[fila, nx + 1:nx + nw] = seg[:-1]               # Un píxel a la derecha.
    cuadro2 = Image.fromarray(b, "RGBA")
    return cuadro1, cuadro2


def preparar_calle(calle):
    """La calle de Pixellab trae una columna clara de marco a la derecha: se cambia por
    la de al lado. Luego se pone en espejo una vez sí y otra no (componer), así los
    bordes que se tocan son siempre iguales."""
    a = np.array(calle)
    a[:, -1] = a[:, -2]
    return Image.fromarray(a, "RGBA")


def componer(piezas):
    """Las dos capas de la cercana (fondo y primer plano) y, para el noren, la lista
    de x donde cae cada puesto. Devuelve (fondo, frente, xs_puesto, casas), con casas =
    [(nombre, x, volteada)] de cada fachada."""
    fondo = Image.new("RGBA", (ANCHO, ALTO), (0, 0, 0, 0))
    frente = Image.new("RGBA", (ANCHO, ALTO), (0, 0, 0, 0))
    calle = piezas["calle"]
    for k, x in enumerate(range(0, ANCHO, calle.width)):
        pegar(fondo, calle, x, SUELO_Y, voltear=bool(k & 1))   # En espejo: sin costura.
    xs, casas, x, i = [], [], 0, 0
    while x < ANCHO:
        nombre, volteada = SECUENCIA[i % len(SECUENCIA)]
        img = piezas[nombre]
        if nombre == "puesto":
            pegar(fondo, img, x, PUESTO_Y)
            xs.append(x)
        else:
            pegar(fondo, img, x, FACHADA_Y, volteada)
            casas.append((nombre, x, volteada))
        xs_inicio = x
        for aldeano, k, dx, volt in ALDEANOS:
            if i % len(SECUENCIA) == k:
                pegar(fondo, piezas[aldeano], xs_inicio + dx, ALDEANO_Y, volt)
        x += img.width
        i += 1
    for fx in FAROLES:
        pegar(frente, piezas["farol"], fx, 224 - piezas["farol"].height)
    for bx in BARRILES:
        pegar(frente, piezas["barriles"], bx, 224 - piezas["barriles"].height)
    return fondo, frente, xs, casas


def fuentes_humo(piezas, casas):
    """De qué punto (x, y del mundo) sale el humo: uno de cada dos tejados, en la
    columna HUMO_X (espejada si la fachada va volteada), justo encima del tejado.
    Ordenadas por x (ambiente.c busca las que están a la vista)."""
    fuentes = []
    for nombre, x, volteada in casas[::2]:
        a = np.array(piezas[nombre])[..., 3]
        col = (a.shape[1] - 1 - HUMO_X) if volteada else HUMO_X
        filas = np.nonzero(a[:, col])[0]
        if len(filas) == 0:
            continue
        fuentes.append((x + col, FACHADA_Y + int(filas[0])))
    return sorted(fuentes)


def cuadros_16(img, quitar_oscuros=False):
    """Una tira de 32x16 → sus dos cuadros de 16x16 en índices de PALETA_OBJETOS. Con
    quitar_oscuros, el contorno oscuro (canal rojo < 0x50) se vuelve transparente: el
    humo no tiene borde."""
    a = np.array(opacar(img))
    if quitar_oscuros:
        a[(a[..., 0] < 0x50) & (a[..., 3] > 0), 3] = 0
    cuadros = []
    for k in range(2):
        pix = a[:16, k * 16:k * 16 + 16].reshape(256, 4)
        ind, _ = indices_tile(pix, PALETA_OBJETOS)
        cuadros.append(ind.reshape(16, 16))
    return cuadros


def tiles_ambiente(cuadros):
    """Las piezas de 16x16 (en orden) → bytes para la VRAM en dos tiras: primero la
    fila de arriba de todas (TL, TR de cada pieza), luego la de abajo (BL, BR). Así
    la pieza k cae en el tile base + 2k y su mitad de abajo en base + 16 + 2k."""
    arriba, abajo = b"", b""
    for c in cuadros:
        arriba += tile_4bpp(list(c[:8, :8].reshape(64))) + tile_4bpp(list(c[:8, 8:].reshape(64)))
        abajo += tile_4bpp(list(c[8:, :8].reshape(64))) + tile_4bpp(list(c[8:, 8:].reshape(64)))
    return arriba + abajo


# --- Conversión a tiles ---------------------------------------------------------------

def indices_tile(pix, paleta):
    """Píxeles RGBA de un tile (64) → (índices 0..15, error). Transparente = 0; el resto,
    el color más cercano de la paleta (índice 1..15)."""
    pal = np.array([hex_rgb(c) for c in paleta], dtype=np.int32)
    rgb = pix[:, :3].astype(np.int32)
    d = ((rgb[:, None, :] - pal[None, :, :]) ** 2 * np.array([3, 4, 2])).sum(axis=2)
    mejor = d.argmin(axis=1)
    opaco = pix[:, 3] > 0
    ind = np.where(opaco, mejor + 1, 0)
    err = int(np.where(opaco, d.min(axis=1), 0).sum())
    return ind.astype(np.uint8), err


def tile_de(img_np, cx, cy, paletas):
    """El tile (cx, cy): (índices, número de paleta en `paletas`)."""
    pix = img_np[cy * 8:cy * 8 + 8, cx * 8:cx * 8 + 8].reshape(64, 4)
    mejor = None
    for k, pal in enumerate(paletas):
        ind, err = indices_tile(pix, pal)
        if mejor is None or err < mejor[2]:
            mejor = (ind, k, err)
    return mejor[0], mejor[1]


def variantes(ind):
    """El tile y sus tres volteos: [(índices, h, v)]."""
    m = ind.reshape(8, 8)
    return [(m, 0, 0), (m[:, ::-1], 1, 0), (m[::-1, :], 0, 1), (m[::-1, ::-1], 1, 1)]


class Juego:
    """Juego de tiles sin repetidos: los volteados cuentan como el mismo tile."""

    def __init__(self):
        self.tiles = [np.zeros(64, np.uint8)]          # El 0: transparente.
        self.paleta = [0]
        self.usos = [0]
        self.clave = {(0, bytes(64)): (0, 0, 0)}

    def reservar(self, ind, pal):
        """Un tile que no se comparte (el noren): siempre uno nuevo."""
        self.tiles.append(ind.copy())
        self.paleta.append(pal)
        self.usos.append(1)
        return len(self.tiles) - 1

    def buscar(self, ind, pal):
        """(tile, h, v): el tile igual (o volteado) que ya está, o uno nuevo."""
        if not ind.any():
            self.usos[0] += 1
            return 0, 0, 0
        for m, h, v in variantes(ind):
            k = (pal, m.tobytes())
            if k in self.clave:
                t, h0, v0 = self.clave[k]
                self.usos[t] += 1
                return t, h ^ h0, v ^ v0
        t = len(self.tiles)
        self.tiles.append(ind.copy())
        self.paleta.append(pal)
        self.usos.append(1)
        self.clave[(pal, ind.tobytes())] = (t, 0, 0)
        return t, 0, 0


def reducir(juego, mapa, limite, fijos):
    """Mientras haya más de `limite` tiles, junta el par más parecido (misma paleta, el
    que menos se usa pasa a ser el otro). Los tiles 0..fijos-1 no se tocan. Devuelve
    cuántos se juntaron. `mapa` es [fila][col] = (tile, pal, prio, h, v) y se reescribe."""
    n = len(juego.tiles)
    if n <= limite:
        return 0
    T = np.array(juego.tiles, dtype=np.int16)
    P = np.array(juego.paleta)
    vivos = np.ones(n, bool)
    vivos[:fijos] = False
    destino = list(range(n))
    juntados = 0
    while vivos.sum() + fijos > limite:
        idx = np.nonzero(vivos)[0]
        sub = T[idx]
        d = (sub[:, None, :] != sub[None, :, :]).sum(axis=2).astype(np.float64)
        d[P[idx][:, None] != P[idx][None, :]] = 1e9
        np.fill_diagonal(d, 1e9)
        a, b = np.unravel_index(d.argmin(), d.shape)
        ta, tb = idx[a], idx[b]
        quitar, queda = (ta, tb) if juego.usos[ta] <= juego.usos[tb] else (tb, ta)
        vivos[quitar] = False
        destino[quitar] = queda
        juego.usos[queda] += juego.usos[quitar]
        juntados += 1
    # Renumerar: los que quedan, en orden.
    def final(t):
        while destino[t] != t:
            t = destino[t]
        return t
    nuevos = {}
    tiles, paletas = [], []
    for t in range(n):
        if final(t) == t:
            nuevos[t] = len(tiles)
            tiles.append(juego.tiles[t])
            paletas.append(juego.paleta[t])
    juego.tiles, juego.paleta = tiles, paletas
    for fila in mapa:
        for c, (t, pal, prio, h, v) in enumerate(fila):
            fila[c] = (nuevos[final(t)], pal, prio, h, v)
    return juntados


def entrada(t, pal, prio, h, v):
    """La palabra del mapa: vhopppcc cccccccc."""
    return (v << 15) | (h << 14) | (prio << 13) | (pal << 10) | t


def convertir(fondo, frente, xs_puesto, noren2):
    """Cercana → (juego, mapa, tiles_noren_cuadro2, primer_noren, juntados)."""
    img = Image.alpha_composite(fondo, frente)
    a = np.array(img)
    prio = np.array(frente)[..., 3] > 0
    paletas = [PALETA_ARQ, PALETA_GENTE]
    juego = Juego()
    mapa = [[None] * COLUMNAS for _ in range(FILAS)]
    # 1) El noren del primer puesto: tiles reservados 1..8; los demás puestos, los mismos.
    nx, ny, nw, nh = NOREN
    celdas = [(cx, cy) for cy in range(nh // 8) for cx in range(nw // 8)]
    n2 = np.array(noren2)
    reservados, cuadro2 = {}, []
    for (cx, cy) in celdas:
        x0 = xs_puesto[0] + nx + cx * 8
        y0 = PUESTO_Y + ny + cy * 8
        ind, pal = tile_de(a, x0 // 8, y0 // 8, paletas)
        reservados[(cx, cy)] = (juego.reservar(ind, pal), pal)
        pix2 = n2[ny + cy * 8:ny + cy * 8 + 8, nx + cx * 8:nx + cx * 8 + 8].reshape(64, 4)
        cuadro2.append(indices_tile(pix2, paletas[pal])[0])
    fijos = len(juego.tiles)
    en_noren = {}
    for xp in xs_puesto:
        for (cx, cy), tp in reservados.items():
            en_noren[((xp + nx) // 8 + cx, (PUESTO_Y + ny) // 8 + cy)] = tp
    # 2) Todo lo demás, columna a columna.
    for cy in range(FILAS):
        for cx in range(COLUMNAS):
            p = int(prio[cy * 8:cy * 8 + 8, cx * 8:cx * 8 + 8].any())
            if (cx, cy) in en_noren and not p:
                t, pal = en_noren[(cx, cy)]
                mapa[cy][cx] = (t, pal, 0, 0, 0)
                continue
            ind, pal = tile_de(a, cx, cy, paletas)
            t, h, v = juego.buscar(ind, pal)
            mapa[cy][cx] = (t, pal, p, h, v)
    juntados = reducir(juego, mapa, LIMITE_TILES, fijos)
    return juego, mapa, cuadro2, 1, juntados


def mapa_columnas(mapa):
    """[fila][col] → bytes columna a columna (32 palabras por columna)."""
    pal_snes = [PALETA_SNES_ARQ, PALETA_SNES_GENTE]
    vals = []
    for cx in range(COLUMNAS):
        for cy in range(FILAS):
            t, pal, p, h, v = mapa[cy][cx]
            vals.append(entrada(t, pal_snes[pal], p, h, v))
    return struct.pack(f"<{len(vals)}H", *vals)


def vista(juego, mapa):
    """La cercana tal como la dibujará la SNES (para mirarla): RGBA."""
    pals = [[(0, 0, 0)] + [hex_rgb(c) for c in PALETA_ARQ],
            [(0, 0, 0)] + [hex_rgb(c) for c in PALETA_GENTE]]
    out = np.zeros((ALTO, ANCHO, 4), np.uint8)
    for cy in range(FILAS):
        for cx in range(COLUMNAS):
            t, pal, p, h, v = mapa[cy][cx]
            m = np.array(juego.tiles[t]).reshape(8, 8)
            if h:
                m = m[:, ::-1]
            if v:
                m = m[::-1, :]
            for y in range(8):
                for x in range(8):
                    k = m[y, x]
                    if k:
                        out[cy * 8 + y, cx * 8 + x] = list(pals[pal][k]) + [255]
    return Image.fromarray(out, "RGBA")


# --- La lejana y el cielo -----------------------------------------------------------

def tapar_costura(img, ancho_monte=88, cima=34, semilla=7):
    """Un monte oscuro centrado en x = 0 (la costura: a su izquierda está el borde
    derecho de la imagen y a su derecha el izquierdo) que tapa la columna de la
    costura entera desde su cima, con laderas irregulares; el resto queda igual."""
    a = np.array(img)
    w, h = img.width, img.height
    rng = np.random.default_rng(semilla)
    ruido = np.cumsum(rng.integers(-1, 2, size=2 * ancho_monte + 1))
    oscuro = list(hex_rgb(PALETA_LEJOS[6])) + [255]     # #241830, el monte cercano.
    borde = list(hex_rgb(PALETA_LEJOS[7])) + [255]      # #3a2848, el filo que da al sol.
    for k, dx in enumerate(range(-ancho_monte, ancho_monte + 1)):
        t = abs(dx) / ancho_monte                          # 0 en la cima, 1 al pie.
        y_top = int(cima + (h - cima) * (t ** 1.4) + ruido[k] * 0.6)
        y_top = max(cima - 4, min(h, y_top))
        x = dx % w
        a[y_top:, x] = oscuro
        if y_top < h and dx < 0:
            a[y_top, x] = borde
    return Image.fromarray(a, "RGBA")


def lejana(paisaje):
    """512x256 indexado: el paisaje subido LEJOS_Y y, debajo, el color más oscuro."""
    p = tapar_costura(opacar(paisaje))
    lienzo = Image.new("RGBA", (512, 256), (0, 0, 0, 0))
    pegar(lienzo, p, 0, LEJOS_Y)
    a = np.array(lienzo)
    fin = p.height + LEJOS_Y
    a[fin:, :] = list(hex_rgb(PALETA_LEJOS[0])) + [255]
    pal = [hex_rgb(c) for c in PALETA_LEJOS]
    ind = np.zeros(a.shape[:2], np.uint8)
    for y in range(a.shape[0]):
        fila, _ = indices_tile(a[y], PALETA_LEJOS)
        ind[y] = fila
    out = Image.fromarray(ind, "P")
    plano = [255, 0, 255]
    for c in pal:
        plano += list(c)
    out.putpalette(plano + [0] * (768 - len(plano)))
    return out


# El cielo solo se ve por encima de las montañas (con LEJOS_Y = -40, de la línea 0 a la
# ~55): el degradado entero, del violeta al naranja, cabe en esa franja.
CIELO = [(0, "#2a1c50"), (14, "#5a2c6e"), (28, "#a03c74"), (40, "#e0606a"),
         (52, "#f89058"), (70, "#f8b868"), (223, "#f8d090")]


def cielo():
    """256x224, un color por línea (gfx4snes -n lee la primera columna)."""
    filas = []
    for y in range(224):
        for (y0, c0), (y1, c1) in zip(CIELO, CIELO[1:]):
            if y0 <= y <= y1:
                t = (y - y0) / max(1, y1 - y0)
                a, b = hex_rgb(c0), hex_rgb(c1)
                rgb = tuple(int(round(a[i] + (b[i] - a[i]) * t)) & 0xF8 for i in range(3))
                break
        filas.append(rgb)
    colores = sorted(set(filas), key=filas.index)
    img = Image.new("P", (256, 224))
    plano = []
    for c in colores:
        plano += list(c)
    img.putpalette(plano + [0] * (768 - len(plano)))
    px = img.load()
    for y, c in enumerate(filas):
        k = colores.index(c)
        for x in range(256):
            px[x, y] = k
    return img


def main():
    faltan = [f"{c}/{n:02d}.png" for c, n in FUENTES.values() if not (ORIGEN / c / f"{n:02d}.png").exists()]
    if faltan:
        sys.exit("ERROR: faltan piezas de Pixellab en " + str(ORIGEN) + ": " + ", ".join(faltan))
    piezas = {k: opacar(cargar(k)) for k in FUENTES}
    piezas["puesto"], noren2 = preparar_puesto(piezas["puesto"], PALETA_ARQ[1])
    piezas["calle"] = preparar_calle(piezas["calle"])
    fondo, frente, xs, casas = componer(piezas)
    juego, mapa, cuadro2, primero, juntados = convertir(fondo, frente, xs, noren2)
    DESTINO.mkdir(parents=True, exist_ok=True)
    (DESTINO / "cerca_til.bin").write_bytes(b"".join(tile_4bpp(list(t)) for t in juego.tiles))
    (DESTINO / "cerca_pal.bin").write_bytes(paleta_bytes(PALETA_ARQ) + paleta_bytes(PALETA_GENTE))
    (DESTINO / "cerca_col.bin").write_bytes(mapa_columnas(mapa))
    (DESTINO / "noren_til.bin").write_bytes(
        b"".join(tile_4bpp(list(juego.tiles[primero + k])) for k in range(len(cuadro2)))
        + b"".join(tile_4bpp(list(t)) for t in cuadro2))
    vista(juego, mapa).save(DESTINO / "cerca_vista.png")
    lejana(piezas["lejana"]).save(DESTINO / "lejos.png")
    cielo().save(DESTINO / "atardecer.bmp")
    cuadros = cuadros_16(piezas["humo"], quitar_oscuros=True) + cuadros_16(piezas["pajaro"])
    (DESTINO / "ambiente_til.bin").write_bytes(tiles_ambiente(cuadros))
    (DESTINO / "ambiente_pal.bin").write_bytes(paleta_bytes(PALETA_OBJETOS))
    humos = fuentes_humo(piezas, casas)
    (DESTINO / "aldea_datos.h").write_text(
        "/* aldea_datos.h — GENERADO por scripts/preparar_aldea.py: no editar a mano.\n"
        "   La capa cercana de la aldea (res/aldea/cerca_*.bin): cuántos tiles tiene y dónde\n"
        "   están los del noren, que el juego cambia cada 8 frames (noren_til.bin: el cuadro 1\n"
        "   y luego el 2, ALDEA_NOREN_TILES tiles cada uno). */\n"
        "#ifndef ALDEA_DATOS_H\n#define ALDEA_DATOS_H\n"
        f"#define ALDEA_TILES {len(juego.tiles)}\n"
        f"#define ALDEA_NOREN_PRIMERO {primero}\n"
        f"#define ALDEA_NOREN_TILES {len(cuadro2)}\n"
        f"#define ALDEA_PALETA {PALETA_SNES_ARQ}          /* La primera de las dos (4 y 5). */\n"
        "/* El humo: de qué punto de los tejados sale (x, y del mundo), ordenados por x. Las\n"
        "   tablas, solo en el archivo que define ALDEA_HUMOS_TABLAS (juego/ambiente.c). */\n"
        f"#define ALDEA_HUMOS {len(humos)}\n"
        "#ifdef ALDEA_HUMOS_TABLAS\n"
        "static const u16 aldea_humo_x[ALDEA_HUMOS] = {" + ", ".join(str(x) for x, _ in humos) + "};\n"
        "static const u16 aldea_humo_y[ALDEA_HUMOS] = {" + ", ".join(str(y) for _, y in humos) + "};\n"
        "#endif\n"
        "#endif\n", encoding="utf-8")
    print(f"cercana: {len(juego.tiles)} tiles (límite {LIMITE_TILES}; juntados {juntados}), "
          f"{len(xs)} puestos, noren {len(cuadro2)} tiles desde el {primero}; humo en {humos}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
