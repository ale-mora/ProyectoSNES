#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    demo_27_aldea.py — Lee la capa cercana de la aldea TAL COMO LA CARGA LA SNES y
    cuenta lo que hay dentro.

    PROPÓSITO
      res/aldea/ guarda la aldea ya en el formato del hardware (lo escribe
      scripts/preparar_aldea.py). Esta demo no usa el script: lee los bytes y los
      interpreta como lo hace la PPU, para ver con números las decisiones de la unidad:
        A. el juego de tiles: cuántos, cuántos bytes, qué paleta usa cada uno;
        B. el mapa: 256 columnas × 32 filas, palabra a palabra (tile, paleta,
           prioridad, volteos), cuántas celdas comparten tile y cuántas van volteadas;
        C. el primer plano: qué celdas llevan el bit de prioridad (las que tapan a los
           personajes) y en qué columnas;
        D. el noren: los tiles reservados y cuánto cambia el segundo cuadro;
        E. lo que cuesta copiarlo: bytes por columna y por frame de cámara;
        F. el humo y los pájaros: de qué tejados sale el humo (aldea_datos.h).

    SE CONECTA CON
      Lee:      res/aldea/cerca_til.bin, cerca_pal.bin, cerca_col.bin, noren_til.bin,
                ambiente_til.bin, aldea_datos.h (versionados).
      Imita:    src/juego/fondo_nivel.c (apuntar_columna, fondo_nivel_noren).

    HARDWARE QUE TOCA
      ninguno (PC).

    VARIABLES: ninguna global.
    Ejecutar (desde esta carpeta):  python demo_27_aldea.py
    → glosario: Tile, Tilemap, Paleta, Streaming de columnas
---------------------------------------------------------------------------------
"""
import re
import struct
from collections import Counter
from pathlib import Path

RAIZ = Path(__file__).resolve().parents[2]
ALDEA = RAIZ / "res" / "aldea"
COLUMNAS, FILAS = 256, 32
BYTES_TILE = 32


def leer_mapa():
    """cerca_col.bin → lista de palabras, columna a columna (32 por columna)."""
    datos = (ALDEA / "cerca_col.bin").read_bytes()
    return struct.unpack(f"<{len(datos) // 2}H", datos)


def campos(palabra):
    """La palabra del mapa vhopppcc cccccccc → (tile, paleta, prioridad, h, v)."""
    return (palabra & 0x3FF, (palabra >> 10) & 7, (palabra >> 13) & 1,
            (palabra >> 14) & 1, (palabra >> 15) & 1)


def main():
    tiles = (ALDEA / "cerca_til.bin").read_bytes()
    paletas = (ALDEA / "cerca_pal.bin").read_bytes()
    mapa = leer_mapa()
    cabecera = (ALDEA / "aldea_datos.h").read_text(encoding="utf-8")
    define = {k: int(v) for k, v in re.findall(r"#define (\w+) (\d+)", cabecera)}

    print("A. El juego de tiles")
    print(f"   {len(tiles)} bytes = {len(tiles) // BYTES_TILE} tiles de 4bpp "
          f"(la VRAM de la capa cercana tiene sitio para 512)")
    print(f"   {len(paletas)} bytes de paleta = {len(paletas) // 32} paletas de 16 colores "
          f"(a la CGRAM desde la paleta {define['ALDEA_PALETA']})")

    print("\nB. El mapa")
    print(f"   {len(mapa)} palabras = {COLUMNAS} columnas x {FILAS} filas "
          f"({len(mapa) * 2} bytes)")
    usos = Counter()
    por_paleta = Counter()
    volteadas = 0
    prioridad_por_col = Counter()
    for k, p in enumerate(mapa):
        t, pal, prio, h, v = campos(p)
        usos[t] += 1
        por_paleta[pal] += 1
        volteadas += (h or v)
        if prio:
            prioridad_por_col[k // FILAS] += 1
    vacias = usos[0]
    print(f"   celdas vacías (tile 0, transparente): {vacias} de {len(mapa)}")
    print(f"   tiles distintos usados: {len(usos)}; después del vacío, el más repetido sale "
          f"{usos.most_common(2)[1][1]} veces (tile {usos.most_common(2)[1][0]})")
    print(f"   celdas que dibujan un tile volteado (h o v): {volteadas} (sin volteos en el mapa, cada imagen espejada necesitaría su propio tile)")
    print("   celdas por paleta: " + ", ".join(f"paleta {k}: {n}" for k, n in sorted(por_paleta.items())))

    print("\nC. El primer plano (bit de prioridad: tapa a los sprites de prioridad 2)")
    print(f"   celdas con prioridad: {sum(prioridad_por_col.values())}")
    grupos, inicio, ultimo = [], None, None
    for c in sorted(prioridad_por_col):
        if inicio is None:
            inicio = ultimo = c
        elif c == ultimo + 1:
            ultimo = c
        else:
            grupos.append((inicio, ultimo))
            inicio = ultimo = c
    if inicio is not None:
        grupos.append((inicio, ultimo))
    for a, b in grupos:
        print(f"   columnas {a:3d}..{b:3d}  (x {a * 8:4d}..{b * 8 + 7:4d})  "
              f"{sum(prioridad_por_col[c] for c in range(a, b + 1))} celdas")

    print("\nD. El noren (animación de tiles)")
    n, primero = define["ALDEA_NOREN_TILES"], define["ALDEA_NOREN_PRIMERO"]
    noren = (ALDEA / "noren_til.bin").read_bytes()
    c1, c2 = noren[:n * BYTES_TILE], noren[n * BYTES_TILE:]
    print(f"   tiles {primero}..{primero + n - 1} reservados; noren_til.bin = {len(noren)} bytes "
          f"(cuadro 1 + cuadro 2)")
    print(f"   el cuadro 1 es igual que los tiles {primero}..{primero + n - 1} de cerca_til.bin: "
          f"{c1 == tiles[primero * BYTES_TILE:(primero + n) * BYTES_TILE]}")
    print(f"   bytes que cambian entre cuadros: {sum(1 for a, b in zip(c1, c2) if a != b)} de {len(c1)}")
    veces = sum(1 for p in mapa if primero <= (p & 0x3FF) < primero + n)
    print(f"   celdas del mapa que los usan: {veces} ({veces // n} puestos): todos ondean a la vez")

    print("\nE. Lo que cuesta copiarlo")
    print(f"   una columna = {FILAS} palabras = {FILAS * 2} bytes: un DMA de la lista")
    print("   la cámara avanza 2 px por frame: una columna nueva cada 4 frames, "
          "y entonces se copian 2 (la que entra por la derecha y la de la izquierda)")
    print(f"   el noren: {n * BYTES_TILE} bytes cada 8 frames")

    print("\nF. El humo y los pájaros")
    xs = re.search(r"aldea_humo_x\[ALDEA_HUMOS\] = \{([^}]*)\}", cabecera).group(1)
    ys = re.search(r"aldea_humo_y\[ALDEA_HUMOS\] = \{([^}]*)\}", cabecera).group(1)
    pares = list(zip(map(int, xs.split(",")), map(int, ys.split(","))))
    print(f"   {define['ALDEA_HUMOS']} tejados con humo: {pares}")
    print("   separaciones: " + ", ".join(str(b[0] - a[0]) for a, b in zip(pares, pares[1:]))
          + " px (todas > 256 - 32: nunca más de 2 a la vista)")
    amb = (ALDEA / "ambiente_til.bin").read_bytes()
    print(f"   ambiente_til.bin = {len(amb)} bytes = {len(amb) // BYTES_TILE} tiles = "
          f"{len(amb) // (4 * BYTES_TILE)} piezas de 16x16 (bocanada, penacho, alas arriba, alas abajo)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
