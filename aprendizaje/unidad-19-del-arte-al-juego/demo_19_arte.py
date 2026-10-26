#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    demo_19_arte.py — Demo de la unidad 19: las cuentas que hay detrás de llevar
    el arte de Pixellab a la SNES, hechas sobre los archivos reales del juego.

    PROPÓSITO
      Cuatro pasos numerados en la salida:
        1. Por qué no se escala el pixel art: el vecino más cercano con un
           factor no entero repite unas columnas y se come otras.
        2. Anclar por el centro del lienzo y por los pies: dónde cae cada
           lienzo de Pixellab (64..104 px) en la celda de 96x64.
        3. Las tiras ya preparadas de res/personajes/tarou/ (versionadas):
           cuadros, piezas de 32x32 no vacías, bytes de ROM, colores, pies.
        4. La rejilla doble: la clave de 4 bits de cada vértice de una rejilla
           de lógica pequeña, como hace scripts/preparar_nivel.py (vestir).

    SE CONECTA CON
      Lee:      res/personajes/tarou/tarou_*.png (las 13 tiras indexadas).
      Modelo:   scripts/preparar_personaje.py (anclar, paleta_comun) y
                scripts/preparar_nivel.py (piezas_por_esquinas, vestir).
      Ejecutar: python aprendizaje/unidad-19-del-arte-al-juego/demo_19_arte.py
                (desde cualquier carpeta: busca res/ dos niveles por encima).

    HARDWARE QUE TOCA
      ninguno (se ejecuta en el PC).

    VARIABLES
      RAIZ        Path   la raíz del repositorio
      CELDA_*     int    96 x 64: la celda del protagonista
      LIENZOS     list   (estado de Pixellab, ancho, alto) medidos en la exportación

    → glosario: Celda, Metasprite, Paleta, Rejilla doble, Hundir los pies
---------------------------------------------------------------------------------
"""
from pathlib import Path

from PIL import Image

RAIZ = Path(__file__).resolve().parents[2]       # aprendizaje/unidad-19.../ -> raíz
CELDA_ANCHO, CELDA_ALTO = 96, 64                  # la celda: 3 x 2 piezas de 32x32
ANIMACIONES = ["reposo", "caminar", "correr", "saltar", "caer", "agachado", "agachado_caminar",
               "desenvainar", "envainar", "atacar", "echarse", "dormido", "despertar"]
# Lienzos de la exportación nativa de Tarou (medidos el 2026-10-22 en frame_000.png;
# las poses fijas de rotations/ miden 64x64).
LIENZOS = [("Idle", 88, 88), ("Walk", 88, 88), ("Jump", 88, 88), ("Run", 96, 96), ("Sneak", 96, 96),
           ("Unsheathe", 96, 84), ("Stand_up", 104, 84), ("Sleep", 76, 76), ("Attack_1 (pose)", 64, 64)]


def paso_1_escalar() -> None:
    """Vecino más cercano: la columna de salida x toma la de entrada int(x / f)."""
    print("1. Escalar con vecino mas cercano (cada numero: cuantas veces sale esa columna)")
    for ancho, nuevo in [(81, 64), (16, 24), (16, 32)]:
        veces = [0] * ancho
        for x in range(nuevo):
            veces[int(x * ancho / nuevo)] += 1    # la columna de entrada que se copia
        f = nuevo / ancho
        print(f"   {ancho} -> {nuevo} px (factor {f:.2f}): {''.join(str(v) for v in veces[:40])}"
              f"{'...' if ancho > 40 else ''}  perdidas: {veces.count(0)}")
    print("   Con factor no entero los pixeles salen desiguales (0, 1 o 2 columnas);")
    print("   con factor 2 todos salen dobles: por eso el arte entra SIN escalar.")


def paso_2_anclar() -> None:
    """La esquina del lienzo en la celda: centro -> columna 48, pies -> fila 63."""
    print("\n2. Anclar cada lienzo en la celda de 96x64 (por el centro y los pies)")
    for estado, w, h in LIENZOS:
        x = (CELDA_ANCHO - w) // 2               # centro del lienzo -> centro de la celda
        print(f"   {estado:16s} lienzo {w:3d}x{h:3d}: esquina en x={x:4d}; "
              f"{'sobra ' + str(-x) + ' px por cada lado (fuera, transparente)' if x < 0 else 'cabe de lado a lado'}")
    print("   Anclar por la ESQUINA moveria al perro de una animacion a otra: lienzos distintos.")


def paso_3_tiras() -> None:
    """Medidas de las tiras indexadas del juego."""
    print("\n3. Las tiras de res/personajes/tarou/ (celda de 96x64, 3 KB por cuadro)")
    print("   animacion         cuadros piezas  bytes  %banco  ancho  pies")
    colores = set()
    for a in ANIMACIONES:
        with Image.open(RAIZ / "res" / "personajes" / "tarou" / f"tarou_{a}.png") as im:
            n = im.width // CELDA_ANCHO
            datos = im.tobytes()
            colores |= set(datos)                # índices de paleta usados
            piezas_max, ancho_max, pies = 0, 0, 0
            for k in range(n):
                cuadro = im.crop((k * CELDA_ANCHO, 0, (k + 1) * CELDA_ANCHO, CELDA_ALTO))
                caja = cuadro.point(lambda i: 255 if i else 0).getbbox()   # sin el índice 0
                if caja:
                    ancho_max = max(ancho_max, caja[2] - caja[0])
                    pies = max(pies, caja[3] - 1)
                piezas = 0
                for py in range(2):                # 2 filas x 3 columnas de 32x32
                    for px in range(3):
                        bloque = cuadro.crop((px * 32, py * 32, px * 32 + 32, py * 32 + 32))
                        piezas += 1 if bloque.getbbox() else 0
                piezas_max = max(piezas_max, piezas)
            print(f"   {a:18s} {n:5d} {piezas_max:6d} {n * 3072:6d}  {n * 3072 * 100 // 32768:4d}%  {ancho_max:5d}  {pies:4d}")
    print(f"   indices de paleta usados en total: {len(colores)} (0 = transparente + 15 colores)")


def paso_4_rejilla_doble() -> None:
    """Clave de cada vértice: TL=1, TR=2, BL=4, BR=8 si esa celda de lógica es tierra."""
    logica = ["........",
              "...##...",
              "########",
              "########"]
    filas, cols = len(logica), len(logica[0])

    def tierra(c, f):                            # fuera: los lados repiten el borde,
        if f < 0:                                # abajo la última fila, arriba vacío
            return 0
        c = min(max(c, 0), cols - 1)
        f = min(f, filas - 1)
        return 1 if logica[f][c] == "#" else 0

    print("\n4. Rejilla doble: la logica (# = solido) y la clave de cada vertice")
    for f, fila in enumerate(logica):
        print(f"   logica fila {f}: {fila}")
    usadas = set()
    for f in range(filas + 1):
        claves = []
        for c in range(cols + 1):
            k = tierra(c - 1, f - 1) | tierra(c, f - 1) << 1 | tierra(c - 1, f) << 2 | tierra(c, f) << 3
            usadas.add(k)
            claves.append(f"{k:2d}")
        print(f"   vertices fila {f} (arte en y={f * 16 - 8:3d}): {' '.join(claves)}")
    print(f"   combinaciones usadas: {sorted(usadas)} de 16; la 0 no se dibuja (aire)")
    print("   Cada celda de arte va en (c*16-8, f*16-8): media celda desplazada, y como 8")
    print("   es multiplo de 8 todo cae en la rejilla de tiles de la SNES.")


if __name__ == "__main__":
    paso_1_escalar()
    paso_2_anclar()
    paso_3_tiras()
    paso_4_rejilla_doble()
