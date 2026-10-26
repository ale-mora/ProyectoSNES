#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    piezas16.py — Tira de cuadros → piezas de 16×16 sin las vacías (formato .p16).

    PROPÓSITO
      El formato de sprites del beat 'em up (docs/diseno-beat-em-up.md §8 v3):
      cada cuadro de una tira PNG indexada se corta en piezas de 16×16 y se
      quedan solo las que tienen algún píxel. En la VRAM las piezas de un cuadro
      van de 8 en 8 por «par de filas» de 1 KB (la pieza k en el tile
      base + (k/8)·32 + (k%8)·2). En el archivo, los tiles de cada cuadro están en
      el ORDEN DE SUBIDA: los pares completos tal cual (1 KB cada uno, como en la
      VRAM) y, si el último par va a medias con q piezas, su fila de arriba
      (q·64 B) y luego la de abajo (q·64 B). Así el plan de copias sale del número
      de piezas, sin tablas (juego/subida.c), y un cuadro de n piezas son
      exactamente n·128 bytes.

      Formato .p16 (little endian), una tira por archivo (≤ 32 KB, un banco):
        índice: un registro de 8 bytes por cuadro (8: índice con desplazamiento)
          u16 desp_piezas   desplazamiento de su tabla de piezas en el archivo
          u16 desp_tiles    desplazamiento de sus tiles en el archivo
          u8  n             piezas del cuadro
          u8  0             relleno
          u16 desp_espejo   desplazamiento de su tabla ESPEJADA (mirando a la izquierda)
        tablas de piezas: por cuadro, n pares (dx, dy) en píxeles dentro de la celda
          y, detrás, los mismos n pares espejados: dx' = ancho - 16 - dx (la pieza
          también se voltea con el bit de la OAM). Así sprites_oam.asm siempre suma
          x0 + dx, sin mirar hacia dónde mira la entidad.
        tiles: n·128 bytes por cuadro, en orden de subida
      El número de cuadros lo sabe el código (tarou_anim.h y similares).

    SE CONECTA CON
      Lo usan:  el Makefile (las tiras de Tarou: res/personajes/tarou/*.png →
                *.p16), scripts/preparar_prueba_carga.py (los ninjas) y
                scripts/pruebas/prueba_piezas16.py.
      Lo leen:  data.asm (.incbin) y src/juego/sprites.c / subida.c.

    HARDWARE QUE TOCA
      ninguno; produce datos con las reglas de la PPU: tile 4bpp de 32 bytes
      (planos 0-1 intercalados por fila y luego 2-3), índice 0 transparente.

    VARIABLES (constantes del módulo)
      PIEZA     int  16    lado de una pieza
      POR_PAR   int  8     piezas por par de filas
      BYTES_PIEZA int 128  4 tiles de 32 bytes
      INDICE    int  8     bytes de un registro del índice

    → glosario: Tile, Metasprite, Pieza de 16×16, Par de filas
---------------------------------------------------------------------------------
"""
import argparse
import struct
import sys

from PIL import Image

PIEZA = 16
POR_PAR = 8
BYTES_TILE = 32
BYTES_PIEZA = 4 * BYTES_TILE
BYTES_PAR = 2 * 16 * BYTES_TILE     # 1024: dos filas de 16 tiles.
INDICE = 8
BANCO = 32768


def pixeles(img):
    """Los píxeles de una imagen en una lista (fila a fila)."""
    return list(img.get_flattened_data()) if hasattr(img, "get_flattened_data") else list(img.getdata())


def tile_4bpp(indices):
    """64 índices (0..15, fila a fila) → 32 bytes del tile 4bpp de la SNES.
    Por fila: bytes 2y y 2y+1 = planos 0 y 1; bytes 16+2y y 17+2y = planos 2 y 3.
    El bit 7 de cada byte es el píxel de la izquierda."""
    datos = bytearray(BYTES_TILE)
    for y in range(8):
        for x in range(8):
            v = indices[y * 8 + x]
            bit = 0x80 >> x
            if v & 1:
                datos[2 * y] |= bit
            if v & 2:
                datos[2 * y + 1] |= bit
            if v & 4:
                datos[16 + 2 * y] |= bit
            if v & 8:
                datos[17 + 2 * y] |= bit
    return bytes(datos)


def piezas_no_vacias(cuadro):
    """Lista de (dx, dy) de las piezas de 16×16 con algún píxel (índice != 0) de una
    imagen indexada, por filas de arriba abajo y de izquierda a derecha."""
    piezas = []
    for py in range(0, cuadro.height, PIEZA):
        for px in range(0, cuadro.width, PIEZA):
            caja = cuadro.crop((px, py, px + PIEZA, py + PIEZA))
            if any(v != 0 for v in pixeles(caja)):
                piezas.append((px, py))
    return piezas


def tile_de_pieza(k):
    """Tile (dentro del bloque del cuadro en la VRAM) de la pieza k."""
    return (k // POR_PAR) * 32 + (k % POR_PAR) * 2


def _tiles_pieza(cuadro, px, py):
    """Los 4 tiles de una pieza: arriba-izquierda, arriba-derecha, abajo-izq., abajo-der."""
    salida = []
    for ty in range(2):
        for tx in range(2):
            caja = cuadro.crop((px + tx * 8, py + ty * 8, px + tx * 8 + 8, py + ty * 8 + 8))
            salida.append(tile_4bpp(pixeles(caja)))
    return salida


def tiles_en_orden_de_subida(cuadro, piezas):
    """Los tiles del cuadro en el orden de subida: cada par completo como en la VRAM
    (fila de arriba: 16 tiles; fila de abajo: 16 tiles) y el último par a medias como
    su fila de arriba (2q tiles) seguida de la de abajo (2q tiles). n·128 bytes."""
    n = len(piezas)
    tiles = [_tiles_pieza(cuadro, px, py) for (px, py) in piezas]
    datos = bytearray()
    for inicio in range(0, n, POR_PAR):
        grupo = tiles[inicio:inicio + POR_PAR]
        for t in grupo:                              # Fila de arriba del par (512 B si
            datos += t[0] + t[1]                     # está completo, q·64 si va a medias).
        for t in grupo:                              # Fila de abajo del par.
            datos += t[2] + t[3]
    assert len(datos) == n * BYTES_PIEZA
    return bytes(datos)


def cuadros_de_tira(tira, ancho, alto):
    """Los cuadros de una tira indexada (en horizontal), cada uno de ancho × alto."""
    return [tira.crop((i * ancho, 0, i * ancho + ancho, alto)) for i in range(tira.width // ancho)]


def construir_p16(cuadros):
    """Bytes del archivo .p16 para una lista de cuadros indexados."""
    n_cuadros = len(cuadros)
    listas = [piezas_no_vacias(c) for c in cuadros]
    tablas = bytearray()
    tiles = bytearray()
    registros = []
    base_tablas = n_cuadros * INDICE
    for c, piezas in zip(cuadros, listas):
        desp = len(tablas)
        for dx, dy in piezas:
            tablas += bytes((dx, dy))
        espejo = len(tablas)
        for dx, dy in piezas:
            tablas += bytes((c.width - PIEZA - dx, dy))
        registros.append((desp, len(tiles), len(piezas), espejo))
        tiles += tiles_en_orden_de_subida(c, piezas)
    if len(tablas) % 2:
        tablas += b"\x00"                            # Los tiles, en dirección par.
    base_tiles = base_tablas + len(tablas)
    indice = bytearray()
    for dp, dt, n, de in registros:
        indice += struct.pack("<HHBBH", base_tablas + dp, base_tiles + dt, n, 0, base_tablas + de)
    datos = bytes(indice + tablas + tiles)
    return datos, listas


def paleta_bgr555(tira):
    """Los 16 primeros colores de la paleta de la tira, en BGR de 5 bits (32 bytes)."""
    pal = (tira.getpalette() or []) + [0] * 48
    salida = bytearray()
    for i in range(16):
        r, g, b = pal[i * 3:i * 3 + 3]
        v = ((b >> 3) << 10) | ((g >> 3) << 5) | (r >> 3)
        salida += bytes((v & 0xFF, v >> 8))
    return bytes(salida)


def main(argv=None):
    p = argparse.ArgumentParser(description="Tira PNG indexada → piezas de 16×16 (.p16).")
    p.add_argument("entrada", help="tira PNG indexada (cuadros en horizontal)")
    p.add_argument("salida", help="archivo .p16")
    p.add_argument("--celda", default="96x64", help="ancho x alto de un cuadro (por defecto 96x64)")
    p.add_argument("--pal", help="escribe además la paleta (32 bytes BGR555)")
    a = p.parse_args(argv)
    ancho, alto = (int(v) for v in a.celda.lower().split("x"))
    tira = Image.open(a.entrada)
    if tira.mode != "P":
        sys.exit("%s no es una imagen indexada" % a.entrada)
    datos, listas = construir_p16(cuadros_de_tira(tira, ancho, alto))
    if len(datos) > BANCO:
        sys.exit("%s: %d bytes, no cabe en un banco de 32 KB" % (a.salida, len(datos)))
    with open(a.salida, "wb") as f:
        f.write(datos)
    if a.pal:
        with open(a.pal, "wb") as f:
            f.write(paleta_bgr555(tira))
    print("%s: %d cuadros, piezas %s, %d bytes" % (a.salida, len(listas), [len(l) for l in listas], len(datos)))


if __name__ == "__main__":
    main()
