#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    preparar_prueba_carga.py — El ninja provisional de la ROM de prueba de carga
    (docs/diseno-beat-em-up.md §9) y las predicciones de la prueba.

    PROPÓSITO
      La prueba de carga mide cuántos enemigos humanos caben en pantalla con
      Tarou. Para que la medida sea honesta, el enemigo provisional ocupa lo que
      ocupará uno de verdad: el samurái humano del pack «FULL_Samurai» (en
      res/descargas/), ampliado ×3 con vecino más cercano (93 px de alto) y
      RECORTADO a la celda decidida de 64×96 (decisión 35), con el cuerpo
      centrado: la espada del ataque se corta en el borde, como saldría pedido a
      Pixellab en esa celda. Andar (8 cuadros) y atacar (7).
      Cada cuadro va en piezas de 16×16 sin las vacías, en el formato .p16 de
      scripts/piezas16.py (el mismo que Tarou). Hasta la v2 de la prueba había
      dos escalas en un lienzo de 128 px de ancho (más ancho que la celda): sus
      cifras están en la spec, §9.1.
      Además calcula, sin la SNES, lo que la ROM debería ver: piezas y bytes por
      cuadro, y las ranuras de 8 px por línea de cada configuración (el límite de
      la PPU es 34). Si la captura de Mesen no coincide, la prueba está mal.

    SE CONECTA CON
      Lee:      res/descargas/graficos/FULL_Samurai 2D Pixel Art v1.2/Sprites/
                WALK.png y ATTACK 1.png (cuadros de 96×96).
      Escribe:  res/prueba_carga/ninja64_andar.bin, ninja64_ataque.bin (.p16),
                ninja_pal.bin (4 paletas: 3 variantes del ninja + la sombra) y
                sombra.bin (2 piezas de 16×16: fila de arriba y de abajo).
      Usa:      scripts/piezas16.py.
      Lo usan:  data.asm (.rodata_prueba_*), src/pantallas/prueba_carga.c y
                scripts/pruebas/prueba_preparar_prueba_carga.py.

    HARDWARE QUE TOCA
      ninguno; produce datos (colores BGR de 5 bits, color 0 transparente).

    VARIABLES (constantes del módulo)
      ESCALA      int  3      ampliación del samurái (31 px → 93 px de alto)
      CELDA_AN    int  64     ancho de la celda del ninja
      CELDA_AL    int  96     alto: los pies en la última fila
      CENTRO_X    int  32     x del cuerpo dentro de la celda

    → glosario: Tile, Paleta, Pieza de 16×16, Ranura por línea, Prueba de carga
---------------------------------------------------------------------------------
"""
import argparse
import colorsys
import os

from PIL import Image

import piezas16
from piezas16 import tile_4bpp, piezas_no_vacias, tile_de_pieza, pixeles  # noqa: F401 (las pruebas)

ESCALA = 3
CELDA_AN = 64
CELDA_AL = 96
CENTRO_X = 32
BYTES_TILE = 32
RANURAS_MAX = 34                    # Trozos de 8 px de sprite por línea (límite de la PPU).
SPRITES_LINEA_MAX = 32

ORIGEN = os.path.join("res", "descargas", "graficos", "FULL_Samurai 2D Pixel Art v1.2", "Sprites")
DESTINO = os.path.join("res", "prueba_carga")


def color_bgr555(r, g, b):
    """Color de 8 bits por canal → palabra BGR de 5 bits (little endian)."""
    v = ((b >> 3) << 10) | ((g >> 3) << 5) | (r >> 3)
    return bytes((v & 0xFF, v >> 8))


# --- Arte ---------------------------------------------------------------------------

def cuadros_de(ruta, n=None):
    """Cuadros de 96×96 de una tira del pack, en RGBA."""
    tira = Image.open(ruta).convert("RGBA")
    total = tira.width // 96
    return [tira.crop((i * 96, 0, i * 96 + 96, 96)) for i in range(total if n is None else n)]


def al_lienzo(cuadro, escala=ESCALA, alto=CELDA_AL, ancho=CELDA_AN, centro=CENTRO_X):
    """Amplía ×escala y lo coloca en un lienzo de ancho × alto: pies abajo, cuerpo
    en x = centro (lo que se sale del lienzo se pierde). El cuerpo del samurái está
    centrado en x≈48 del cuadro original (medido en IDLE y WALK) y sus pies en y=81."""
    grande = cuadro.resize((96 * escala, 96 * escala), Image.NEAREST)
    lienzo = Image.new("RGBA", (ancho, alto), (0, 0, 0, 0))
    lienzo.paste(grande, (centro - 48 * escala, alto - 82 * escala), grande)  # Con máscara.
    return lienzo


def indexar(lienzos):
    """Una paleta de 15 colores para todos los cuadros; índice 0 = transparente."""
    opacos = [c for l in lienzos for c in pixeles(l) if c[3] >= 128]
    muestra = Image.new("RGB", (len(opacos), 1))
    muestra.putdata([c[:3] for c in opacos])
    pal = muestra.quantize(colors=15, method=Image.MEDIANCUT).getpalette()[:45]
    colores = [tuple(pal[i:i + 3]) for i in range(0, 45, 3)]
    cache = {}
    resultado = []
    for l in lienzos:
        datos = []
        for c in pixeles(l):
            if c[3] < 128:
                datos.append(0)
                continue
            if c not in cache:
                cache[c] = 1 + min(range(15), key=lambda i: sum((a - b) ** 2 for a, b in zip(c[:3], colores[i])))
            datos.append(cache[c])
        ind = Image.new("P", l.size, 0)
        ind.putdata(datos)
        resultado.append(ind)
    return colores, resultado


def variante(colores, giro):
    """La misma paleta con el tono girado: otra variante de enemigo sin arte nuevo."""
    salida = []
    for r, g, b in colores:
        h, l, s = colorsys.rgb_to_hls(r / 255, g / 255, b / 255)
        r2, g2, b2 = colorsys.hls_to_rgb((h + giro) % 1.0, l, max(s, 0.5) if l > 0.15 else s)
        salida.append((int(r2 * 255), int(g2 * 255), int(b2 * 255)))
    return salida


def paleta_bytes(colores):
    return b"\x00\x00" + b"".join(color_bgr555(*c) for c in colores)


def sombra():
    """Óvalo negro de 32×16 en 2 piezas de 16×16, como 2 filas de 4 tiles (256 bytes):
    fila de arriba (tiles 0..3) y de abajo (4..7); la pieza 0 es la izquierda."""
    img = Image.new("P", (32, 16), 0)
    for y in range(16):
        for x in range(32):
            if ((x - 15.5) / 15.5) ** 2 + ((y - 9.5) / 5.5) ** 2 <= 1.0:
                img.putpixel((x, y), 1)
    datos = bytearray()
    for fila in range(2):
        for col in range(4):
            caja = img.crop((col * 8, fila * 8, col * 8 + 8, fila * 8 + 8))
            datos += tile_4bpp(pixeles(caja))
    return bytes(datos)


# --- Predicciones -------------------------------------------------------------------

def ranuras_por_linea(entidades):
    """entidades: lista de (y_arriba, piezas) con piezas = [(dy, ancho)]; devuelve
    (ranuras, sprites) por línea de pantalla 0..223."""
    ranuras = [0] * 224
    sprites = [0] * 224
    for y0, piezas in entidades:
        for dy, ancho in piezas:
            for l in range(y0 + dy, y0 + dy + ancho):
                if 0 <= l < 224:
                    ranuras[l] += ancho // 8
                    sprites[l] += 1
    return ranuras, sprites


def peor_cuadro(tabla):
    """El cuadro que más piezas pone en una misma fila de 16 px."""
    return max(tabla, key=lambda piezas: max(sum(1 for (_, dy) in piezas if dy == f) for f in range(0, 96, 16)))


CONFIGURACIONES = [                  # (nombre, desplazamientos en y de los ninjas, sombras)
    ("Tarou solo", [], False),
    ("Tarou + 1, sombras", [0], True),
    ("Tarou + 2, sombras", [0, 0], True),
    ("Tarou + 3 misma y, sombras", [0, 0, 0], True),
    ("Tarou + 3 repartidos, sombras", [0, 12, 24], True),
    ("Tarou + 3 repartidos, sin sombras", [0, 12, 24], False),
]


def predecir(titulo, cuadro, tarou, alto=CELDA_AL, suelo_y=176):
    """Imprime las ranuras por línea de las configuraciones con un cuadro del ninja y
    las piezas de Tarou (lista de dy de sus piezas de 16)."""
    ninja = [(dy, 16) for (_, dy) in cuadro]
    sombra_p = [(0, 16), (0, 16)]
    print("  %s (%d piezas)" % (titulo, len(cuadro)))
    for nombre, desp, con_sombra in CONFIGURACIONES:
        ent = [(suelo_y - 64, tarou)]
        if con_sombra:
            ent.append((suelo_y - 8, sombra_p))
        for d in desp:
            ent.append((suelo_y + d - alto, ninja))
            if con_sombra:
                ent.append((suelo_y + d - 8, sombra_p))
        ran, spr = ranuras_por_linea(ent)
        pasadas = [l for l in range(224) if ran[l] > RANURAS_MAX]
        print("    %-34s máx %2d ranuras, %2d sprites; líneas > %d: %s" % (
            nombre, max(ran), max(spr), RANURAS_MAX,
            ("%d..%d (%d)" % (pasadas[0], pasadas[-1], len(pasadas))) if pasadas else "ninguna"))


def piezas_tarou_peor():
    """Las piezas (dy, 16) del cuadro de andar de Tarou con más piezas en una fila."""
    ruta = os.path.join("res", "personajes", "tarou", "tarou_caminar.png")
    tira = Image.open(ruta)
    cuadros = piezas16.cuadros_de_tira(tira, 96, 64)
    peor = peor_cuadro([piezas_no_vacias(c) for c in cuadros])
    return [(dy, 16) for (_, dy) in peor]


# --- Programa -----------------------------------------------------------------------

def main(argv=None):
    p = argparse.ArgumentParser(description="Ninja provisional y predicciones de la prueba de carga.")
    p.add_argument("--origen", default=ORIGEN)
    p.add_argument("--destino", default=DESTINO)
    a = p.parse_args(argv)
    os.makedirs(a.destino, exist_ok=True)

    andar = cuadros_de(os.path.join(a.origen, "WALK.png"), 8)
    ataque = cuadros_de(os.path.join(a.origen, "ATTACK 1.png"))
    colores, indexados = indexar([al_lienzo(c) for c in andar + ataque])
    tablas = []
    for nombre, cuadros in (("ninja64_andar.bin", indexados[:len(andar)]),
                            ("ninja64_ataque.bin", indexados[len(andar):])):
        datos, listas = piezas16.construir_p16(cuadros)
        with open(os.path.join(a.destino, nombre), "wb") as f:
            f.write(datos)
        tablas.append(listas)
        print("%s: %d cuadros, piezas %s, %d bytes" % (nombre, len(listas), [len(l) for l in listas], len(datos)))

    with open(os.path.join(a.destino, "ninja_pal.bin"), "wb") as f:
        f.write(paleta_bytes(colores))
        f.write(paleta_bytes(variante(colores, 0.33)))
        f.write(paleta_bytes(variante(colores, 0.66)))
        f.write(b"\x00\x00" * 16)                        # Paleta 4: la sombra (color 1 negro).
    with open(os.path.join(a.destino, "sombra.bin"), "wb") as f:
        f.write(sombra())

    todas = tablas[0] + tablas[1]
    print("máximo: %d piezas = %d bytes exactos (el hueco tiene 24)" % (
        max(len(t) for t in todas), max(len(t) for t in todas) * 128))
    tarou = piezas_tarou_peor()
    print("PREDICCIÓN (límite %d ranuras de 8 px y %d sprites por línea; Tarou en piezas de 16, %d)" % (
        RANURAS_MAX, SPRITES_LINEA_MAX, len(tarou)))
    predecir("andando: peor cuadro de andar", peor_cuadro(tablas[0]), tarou)
    predecir("atacando: peor cuadro de ataque", peor_cuadro(tablas[1]), tarou)


if __name__ == "__main__":
    main()
