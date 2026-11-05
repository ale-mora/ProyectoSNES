#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    ejercicio_27_tiles.py — SOLUCIÓN de la parte A del ejercicio 27: un juego de
    tiles sin repetidos, contando los volteados como el mismo tile.

    PROPÓSITO
      Ver el esqueleto (../ejercicio_27_tiles.py). La clave del diccionario es
      (paleta, tile): así encontrar un tile cuesta lo mismo con 10 que con 500.

    SE CONECTA CON
      Lo prueba: prueba_27_tiles.py.
      Modelo:    scripts/preparar_aldea.py (variantes, Juego.buscar).

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: ninguna global.
    → glosario: Tile, Tilemap
---------------------------------------------------------------------------------
"""

VACIO = (0,) * 64


def voltear_h(t):
    """El tile en espejo horizontal (cada fila, al revés)."""
    return tuple(t[y * 8 + (7 - x)] for y in range(8) for x in range(8))


def voltear_v(t):
    """El tile boca abajo (las filas, en orden inverso)."""
    return tuple(t[(7 - y) * 8 + x] for y in range(8) for x in range(8))


class Juego:
    """Tiles distintos (con su paleta) en el orden en que aparecieron."""

    def __init__(self):
        self.tiles = [VACIO]                 # El 0: transparente.
        self.paletas = [0]
        self.clave = {}                      # (paleta, tile) → (número, h, v)

    def buscar(self, t, paleta):
        """(número de tile, h, v) que dibuja `t` con `paleta`; lo añade si es nuevo."""
        if t == VACIO:
            return 0, 0, 0                   # El transparente: siempre el 0.
        variantes = [(t, 0, 0), (voltear_h(t), 1, 0), (voltear_v(t), 0, 1),
                     (voltear_h(voltear_v(t)), 1, 1)]
        for m, h, v in variantes:            # ¿Está ya, tal cual o volteado?
            if (paleta, m) in self.clave:
                n, h0, v0 = self.clave[(paleta, m)]
                return n, h ^ h0, v ^ v0     # Volteo para pasar del guardado a t.
        self.tiles.append(t)                 # Nuevo.
        self.paletas.append(paleta)
        n = len(self.tiles) - 1
        self.clave[(paleta, t)] = (n, 0, 0)
        return n, 0, 0
