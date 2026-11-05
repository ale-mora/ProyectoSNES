#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    ejercicio_27_tiles.py — ESQUELETO de la parte A del ejercicio 27: un juego de
    tiles sin repetidos, contando los volteados como el mismo tile.

    PROPÓSITO
      Es la idea de la clase Juego de scripts/preparar_aldea.py, sin numpy: un tile
      es una tupla de 64 índices de color (0..15, fila a fila, 8x8) y va con el número
      de la paleta que usa. buscar() devuelve el tile del juego que lo dibuja y con qué
      volteo (h, v); si no hay ninguno, lo añade. El tile 0 es el transparente y ya
      está al crear el juego. Completa los TODO **después** de escribir las pruebas
      (prueba_27_tiles.py, ver ejercicio_27.md).

    SE CONECTA CON
      Lo prueba: prueba_27_tiles.py (la escribes tú).
      Modelo:    scripts/preparar_aldea.py (variantes, Juego.buscar).

    HARDWARE QUE TOCA
      ninguno; el mapa de la SNES guarda en cada celda el tile y dos bits de volteo
      (bit 14 horizontal, bit 15 vertical): un tile espejado no ocupa VRAM.

    VARIABLES: ninguna global.
    → glosario: Tile, Tilemap
---------------------------------------------------------------------------------
"""

VACIO = (0,) * 64


def voltear_h(t):
    """El tile en espejo horizontal (cada fila, al revés)."""
    # TODO 3: devuelve una tupla de 64 con las 8 filas invertidas de izquierda a derecha.
    return t


def voltear_v(t):
    """El tile boca abajo (las filas, en orden inverso)."""
    # TODO 4: devuelve una tupla de 64 con las filas de abajo arriba.
    return t


class Juego:
    """Tiles distintos (con su paleta) en el orden en que aparecieron."""

    def __init__(self):
        self.tiles = [VACIO]                 # El 0: transparente.
        self.paletas = [0]
        self.clave = {}                      # (paleta, tile) → (número, h, v)

    def buscar(self, t, paleta):
        """(número de tile, h, v) que dibuja `t` con `paleta`; lo añade si es nuevo."""
        # TODO 5: si t es el vacío, (0, 0, 0) sin añadir nada.
        # TODO 6: prueba t, voltear_h(t), voltear_v(t) y los dos volteos: si alguno está
        #         en self.clave con ESA paleta, devuelve su número y el volteo que hace
        #         falta para pasar del guardado a t.
        # TODO 7: si no, añádelo (tiles, paletas, clave) y devuelve (nuevo, 0, 0).
        self.tiles.append(t)
        self.paletas.append(paleta)
        return len(self.tiles) - 1, 0, 0
