#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    ejercicio_19_cuentas.py — Ejercicio de PC de la unidad 19 (con TODO): las
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
    # TODO 1: una línea con | y <<.
    del tl, tr, bl, br                           # (quita esta línea al resolverlo)
    return 0


def anclar(ancho: int, alto: int, pies: int) -> tuple:
    """TODO 2: (x, y) de la esquina superior izquierda del lienzo dentro de la
    celda. `pies` es la fila (exclusiva) donde acaban los pies en el lienzo: la
    caja opaca acaba ahí, y debe acabar en CELDA_ALTO. `alto` no hace falta: se
    ancla por los pies, no por el borde del lienzo."""
    del alto                                     # (no se usa: a propósito)
    # TODO 2: dos restas (y una división entera por 2).
    del ancho, pies                              # (quita esta línea al resolverlo)
    return (0, 0)


def tiles_distintos(img: list) -> int:
    """TODO 3: img es una lista de filas (listas de índices), de alto y ancho
    múltiplos de 8. Cuenta tiles de 8x8 distintos; un tile y sus tres volteos
    (horizontal, vertical, los dos) cuentan como uno."""
    # TODO 3: recorre los tiles; de cada uno, sus 4 variantes (tuplas de filas);
    # guarda en un set la menor (min) y devuelve cuántas hay.
    del img                                      # (quita esta línea al resolverlo)
    return 0


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
