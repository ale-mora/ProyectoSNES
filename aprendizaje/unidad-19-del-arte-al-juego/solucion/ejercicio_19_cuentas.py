#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    ejercicio_19_cuentas.py — SOLUCIÓN del ejercicio de PC de la unidad 19: las
    tres cuentas de la integración del arte, escritas a mano.

    PROPÓSITO
      TODO 1  clave(): la clave de 4 bits de un vértice de la rejilla doble
              (TL=1, TR=2, BL=4, BR=8), como en scripts/preparar_nivel.py.
      TODO 2  anclar(): dónde va la esquina de un lienzo de Pixellab dentro de
              la celda de 96x64 (centro -> columna 48, pies -> última fila).
      TODO 3  tiles_distintos(): cuántos tiles de 8x8 distintos tiene una
              imagen, contando como iguales los volteados (la SNES los voltea
              gratis), como tiles_distintos() del script.
      Cada parte imprime su resultado; la salida esperada está al final.

    SE CONECTA CON
      Modelo:   scripts/preparar_nivel.py (vestir, tiles_distintos) y
                scripts/preparar_personaje.py (anclar).
      Ejecutar: python ejercicio_19_cuentas.py   (sin Pillow: solo listas)

    HARDWARE QUE TOCA
      ninguno.

    SALIDA ESPERADA (exacta)
      1. claves: 0 15 12 3 8 4 2 1 9 6
      2. 88x88 pies 72 -> (4, -8) | 104x84 pies 69 -> (-4, -5) | 64x64 pies 60 -> (16, 4)
      3. tiles: 1 2 3
---------------------------------------------------------------------------------
"""

CELDA_ANCHO, CELDA_ALTO = 96, 64


def clave(tl: bool, tr: bool, bl: bool, br: bool) -> int:
    """TODO 1: suma 1, 2, 4 y 8 por cada esquina que tiene tierra."""
    return int(tl) | int(tr) << 1 | int(bl) << 2 | int(br) << 3


def anclar(ancho: int, alto: int, pies: int) -> tuple:
    """TODO 2: (x, y) de la esquina superior izquierda del lienzo dentro de la
    celda. `pies` es la fila (exclusiva) donde acaban los pies en el lienzo: la
    caja opaca acaba ahí, y debe acabar en CELDA_ALTO. `alto` no hace falta: se
    ancla por los pies, no por el borde del lienzo."""
    del alto                                     # (no se usa: a propósito)
    return ((CELDA_ANCHO - ancho) // 2, CELDA_ALTO - pies)


def tiles_distintos(img: list) -> int:
    """TODO 3: img es una lista de filas (listas de índices), de alto y ancho
    múltiplos de 8. Cuenta tiles de 8x8 distintos; un tile y sus tres volteos
    (horizontal, vertical, los dos) cuentan como uno."""
    vistos = set()
    for ty in range(len(img) // 8):
        for tx in range(len(img[0]) // 8):
            filas = [tuple(img[ty * 8 + y][tx * 8: tx * 8 + 8]) for y in range(8)]
            variantes = [tuple(filas), tuple(f[::-1] for f in filas),
                         tuple(filas[::-1]), tuple(f[::-1] for f in filas[::-1])]
            vistos.add(min(variantes))           # la misma "forma" sea cual sea el volteo
    return len(vistos)


def imagen(ancho: int, alto: int, puntos: list) -> list:
    """Una imagen de índices 0 con un 1 en cada (x, y) de `puntos`."""
    img = [[0] * ancho for _ in range(alto)]
    for x, y in puntos:
        img[y][x] = 1
    return img


if __name__ == "__main__":
    casos = [(0, 0, 0, 0), (1, 1, 1, 1), (0, 0, 1, 1), (1, 1, 0, 0), (0, 0, 0, 1),
             (0, 0, 1, 0), (0, 1, 0, 0), (1, 0, 0, 0), (1, 0, 0, 1), (0, 1, 1, 0)]
    print("1. claves: " + " ".join(str(clave(*c)) for c in casos))
    print("2. " + " | ".join(f"{w}x{h} pies {p} -> {anclar(w, h, p)}"
                             for w, h, p in [(88, 88, 72), (104, 84, 69), (64, 64, 60)]))
    a = imagen(16, 8, [(0, 0), (15, 0)])         # el mismo tile, volteado: 1
    b = imagen(16, 8, [(0, 0), (14, 7)])         # (6, 7) no es ningún volteo de (0, 0): 2
    c = imagen(24, 8, [(0, 0), (9, 1), (19, 3)]) # tres posiciones distintas
    print(f"3. tiles: {tiles_distintos(a)} {tiles_distintos(b)} {tiles_distintos(c)}")
