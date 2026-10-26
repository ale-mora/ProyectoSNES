#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    mapa_a_columnas.py — Reordena un mapa de gfx4snes (por filas) en columnas,
    para que el juego copie a la VRAM una columna entera con UN solo DMA.

    PROPÓSITO
      gfx4snes guarda el mapa de una imagen de W tiles de ancho y H de alto fila
      a fila: la entrada (x, y) está en la palabra y*W + x. Cuando la cámara
      avanza, el juego necesita la COLUMNA x (H palabras separadas W entre sí),
      y el DMA de la SNES solo lee bytes seguidos. Este script escribe el mismo
      mapa columna a columna: la columna x ocupa las palabras x*H .. x*H + H-1.

    SE CONECTA CON
      Lo usa:   el Makefile (res/nivel1/fondo_cerca.col desde fondo_cerca.map).
      Lo leen:  data.asm (nivel1_cerca_col) y src/juego/fondo_nivel.c (el DMA
                de columnas); scripts/pruebas/prueba_preparar_nivel.py.

    HARDWARE QUE TOCA
      ninguno; produce datos (palabras de 16 bits little-endian, como la VRAM).

    VARIABLES: ninguna global.
    Uso: python scripts/mapa_a_columnas.py ENTRADA.map SALIDA.col --ancho 256
    → glosario: Tilemap, DMA, Streaming de columnas
---------------------------------------------------------------------------------
"""
import argparse
import struct
import sys
from pathlib import Path


def a_columnas(datos: bytes, ancho: int) -> bytes:
    """Mapa por filas (ancho tiles) → mapa por columnas. Cada entrada son 2 bytes."""
    n = len(datos) // 2
    if n % ancho:
        raise ValueError(f"el mapa tiene {n} entradas, que no es múltiplo del ancho {ancho}")
    alto = n // ancho
    pal = struct.unpack(f"<{n}H", datos[: n * 2])
    return struct.pack(f"<{n}H", *(pal[y * ancho + x] for x in range(ancho) for y in range(alto)))


def main() -> int:
    ap = argparse.ArgumentParser(description="Mapa de gfx4snes por filas → por columnas.")
    ap.add_argument("entrada", type=Path)
    ap.add_argument("salida", type=Path)
    ap.add_argument("--ancho", type=int, required=True, help="ancho del mapa en tiles de 8 px")
    args = ap.parse_args()
    try:
        args.salida.write_bytes(a_columnas(args.entrada.read_bytes(), args.ancho))
    except ValueError as e:
        sys.exit(f"ERROR: {args.entrada}: {e}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
