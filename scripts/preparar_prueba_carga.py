#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    preparar_prueba_carga.py — Ninjas provisionales para la ROM de prueba de
    carga (docs/diseno-beat-em-up.md §9) y las predicciones de la prueba.

    PROPÓSITO
      La prueba de carga mide cuántos enemigos humanos caben en pantalla antes
      de pedir su arte a Pixellab. Para que la medida sea honesta, el enemigo
      provisional tiene que ocupar lo que ocupará uno de verdad. Por eso no es
      un rectángulo: es el samurái humano del pack «FULL_Samurai» (en
      res/descargas/), ampliado con vecino más cercano, con sus cuadros de
      ATAQUE, que son los que llenan más piezas porque la espada sale del
      cuerpo. Dos escalas: ×3 (93 px de alto, un humano en una celda de 64×96)
      y ×2 (62 px, un enemigo más pequeño, para comparar).
      Cada cuadro se corta en piezas de 16×16 y se descartan las vacías
      (opción A de la spec §8): solo se guardan y se suben las que tienen algún
      píxel. Las piezas de un cuadro se empaquetan de 8 en 8 en «pares de
      filas» de 1 KB, el formato exacto de la VRAM (16 tiles de ancho; un
      sprite de 16×16 en el tile N usa N, N+1, N+16 y N+17). Así un cuadro sube
      con un solo DMA.
      Además calcula, sin la SNES, lo que la ROM debería ver: piezas y tiles por
      cuadro, y las ranuras de 8 px por línea de cada configuración (el límite
      de la PPU es 34). Si la captura de Mesen no coincide con esto, la prueba
      está mal.

    SE CONECTA CON
      Lee:      res/descargas/graficos/FULL_Samurai 2D Pixel Art v1.2/Sprites/
                WALK.png y ATTACK 1.png (cuadros de 96×96).
      Escribe:  res/prueba_carga/ninjaE_andar.bin, ninjaE_ataque.bin (tiles 4bpp;
                E = 3 o 2, la escala), ninja_pal.bin (4 paletas: 3 variantes del ninja
                + la sombra), sombra.bin (2 piezas de 16×16) y ninja_datos.h
                (las tablas de las dos escalas).
      Lo usan:  el Makefile (regla de res/prueba_carga/), data.asm
                (.rodata_prueba_*), src/pantallas/prueba_carga.c y
                scripts/pruebas/prueba_preparar_prueba_carga.py.

    HARDWARE QUE TOCA
      ninguno; produce datos con las reglas de la PPU: tile 4bpp de 32 bytes
      (planos 0-1 intercalados por fila y luego 2-3), color 0 transparente,
      colores BGR de 5 bits.

    VARIABLES (constantes del módulo)
      ESCALAS     dict {3: 96, 2: 64}  ampliación del samurái → alto del lienzo
      LIENZO_AN   int  128    ancho del lienzo del enemigo (8 piezas)
      CENTRO_X    int  40     x del cuerpo dentro del lienzo (la espada va a la derecha)
      PIEZA       int  16     lado de una pieza
      POR_PAR     int  8      piezas por par de filas de la VRAM (1 KB)

    → glosario: Tile, Paleta, Sprite, Ranura por línea, Prueba de carga
---------------------------------------------------------------------------------
"""
import argparse
import colorsys
import math
import os
import sys

from PIL import Image

ESCALAS = {3: 96, 2: 64}
LIENZO_AN = 128
CENTRO_X = 40
PIEZA = 16
POR_PAR = 8
BYTES_TILE = 32
BYTES_PAR = 1024                    # 2 filas de 16 tiles de 32 bytes.
RANURAS_MAX = 34                    # Trozos de 8 px de sprite por línea (límite de la PPU).
SPRITES_LINEA_MAX = 32

ORIGEN = os.path.join("res", "descargas", "graficos", "FULL_Samurai 2D Pixel Art v1.2", "Sprites")
DESTINO = os.path.join("res", "prueba_carga")


# --- Codificación de la SNES ------------------------------------------------------

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


def color_bgr555(r, g, b):
    """Color de 8 bits por canal → palabra BGR de 5 bits (little endian)."""
    v = ((b >> 3) << 10) | ((g >> 3) << 5) | (r >> 3)
    return bytes((v & 0xFF, v >> 8))


def pixeles(img):
    """Los píxeles de una imagen en una lista (fila a fila)."""
    return list(img.get_flattened_data()) if hasattr(img, "get_flattened_data") else list(img.getdata())


# --- Piezas -------------------------------------------------------------------------

def piezas_no_vacias(cuadro):
    """Lista de (dx, dy) de las piezas de 16×16 con algún píxel (índice != 0) de una
    imagen indexada, por filas de arriba abajo."""
    piezas = []
    for py in range(0, cuadro.height, PIEZA):
        for px in range(0, cuadro.width, PIEZA):
            caja = cuadro.crop((px, py, px + PIEZA, py + PIEZA))
            if any(v != 0 for v in pixeles(caja)):
                piezas.append((px, py))
    return piezas


def tile_de_pieza(k):
    """Tile (dentro del bloque del cuadro) de la pieza k: 8 piezas por par de filas,
    cada una de 2 tiles de ancho: par k//8, columna 2*(k%8)."""
    return (k // POR_PAR) * 32 + (k % POR_PAR) * 2


def empaquetar(cuadro, piezas):
    """Bytes del cuadro en pares de filas de 1 KB, cada pieza en su tile_de_pieza."""
    pares = (len(piezas) + POR_PAR - 1) // POR_PAR
    datos = bytearray(pares * BYTES_PAR)
    for k, (px, py) in enumerate(piezas):
        base = tile_de_pieza(k)
        for ty in range(2):
            for tx in range(2):
                caja = cuadro.crop((px + tx * 8, py + ty * 8, px + tx * 8 + 8, py + ty * 8 + 8))
                n = base + ty * 16 + tx
                datos[n * BYTES_TILE:(n + 1) * BYTES_TILE] = tile_4bpp(pixeles(caja))
    return bytes(datos)


# --- Arte ---------------------------------------------------------------------------

def cuadros_de(ruta, n=None):
    """Cuadros de 96×96 de una tira del pack, en RGBA."""
    tira = Image.open(ruta).convert("RGBA")
    total = tira.width // 96
    return [tira.crop((i * 96, 0, i * 96 + 96, 96)) for i in range(total if n is None else n)]


def al_lienzo(cuadro, escala, alto, ancho=LIENZO_AN, centro=CENTRO_X):
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
    ("Tarou + 3 misma y, sin sombras", [0, 0, 0], False),
    ("Tarou + 4 repartidos, sombras", [0, 8, 16, 24], True),
]


def predecir(titulo, cuadro, alto, suelo_y=176):
    """Imprime las ranuras por línea de las configuraciones con un cuadro del ninja.
    Tarou cuenta como sus 6 piezas de 32×32 (el peor caso)."""
    tarou = [(0, 32), (0, 32), (0, 32), (32, 32), (32, 32), (32, 32)]
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
        print("    %-32s máx %2d ranuras, %2d sprites; líneas > %d: %s" % (
            nombre, max(ran), max(spr), RANURAS_MAX,
            ("%d..%d (%d)" % (pasadas[0], pasadas[-1], len(pasadas))) if pasadas else "ninguna"))


# --- Programa -----------------------------------------------------------------------

def escribir_tablas(f, prefijo, tabla, grupos, alto):
    """Las tablas de una escala: piezas de cada cuadro (dx, dy), tira y desplazamiento."""
    P = prefijo.upper()
    piezas_max = max(len(p) for p in tabla)
    paso = 1 << math.ceil(math.log2(piezas_max))        # Potencia de 2: índice con desplazamiento.
    lineas = [
        "#define %s_CUADROS %d" % (P, len(tabla)),
        "#define %s_PIEZAS_MAX %d       /* El cuadro con más piezas.                     */" % (P, piezas_max),
        "#define %s_PIEZAS_PASO %d      /* Potencia de 2 >= PIEZAS_MAX: fila de la tabla. */" % (P, paso),
        "#define %s_PIEZAS_PASO_BITS %d" % (P, int(math.log2(paso))),
        "#define %s_PARES_MAX %d        /* Pares de filas (1 KB) del cuadro más grande.  */" % (
            P, max(g[2] for g in grupos) // BYTES_PAR),
        "#define %s_ALTO %d             /* Alto del lienzo: los pies en su última fila.  */" % (P, alto),
        "",
        "static const u8 %s_n_piezas[%s_CUADROS] = {%s};" % (prefijo, P, ", ".join(str(len(p)) for p in tabla)),
        "static const u8 %s_tira[%s_CUADROS] = {%s};" % (prefijo, P, ", ".join(str(g[0]) for g in grupos)),
        "static const u16 %s_desp[%s_CUADROS] = {%s};" % (prefijo, P, ", ".join(str(g[1]) for g in grupos)),
        "static const u16 %s_bytes[%s_CUADROS] = {%s};" % (prefijo, P, ", ".join(str(g[2]) for g in grupos)),
        "/* dx, dy de cada pieza; %s_PIEZAS_PASO entradas por cuadro (las que sobran, a 0). */" % P,
        "static const u8 %s_piezas[%s_CUADROS * %s_PIEZAS_PASO * 2] = {" % (prefijo, P, P),
    ]
    for piezas in tabla:
        fila = []
        for k in range(paso):
            dx, dy = piezas[k] if k < len(piezas) else (0, 0)
            fila += [dx, dy]
        lineas.append("    " + ", ".join(str(v) for v in fila) + ",")
    lineas += ["};", "", ""]
    f.write("\n".join(lineas))


def generar(origen, destino, escala, alto, f_tablas):
    """Una escala del ninja: tiras, tablas y el informe. Devuelve los colores."""
    prefijo = "ninja%d" % escala
    andar = cuadros_de(os.path.join(origen, "WALK.png"), 8)
    ataque = cuadros_de(os.path.join(origen, "ATTACK 1.png"))
    colores, indexados = indexar([al_lienzo(c, escala, alto) for c in andar + ataque])

    tabla, grupos, tiras = [], [], [bytearray(), bytearray()]
    for i, cuadro in enumerate(indexados):
        piezas = piezas_no_vacias(cuadro)
        datos = empaquetar(cuadro, piezas)
        tira = 0 if i < len(andar) else 1
        grupos.append((tira, len(tiras[tira]), len(datos)))
        tiras[tira] += datos
        tabla.append(piezas)
    for sufijo, datos in (("_andar.bin", tiras[0]), ("_ataque.bin", tiras[1])):
        if len(datos) > 32768:
            sys.exit("%s%s ocupa %d bytes: no cabe en un banco de 32 KB" % (prefijo, sufijo, len(datos)))
        with open(os.path.join(destino, prefijo + sufijo), "wb") as f:
            f.write(datos)
    escribir_tablas(f_tablas, prefijo, tabla, grupos, alto)

    print("\n%s: samurái ×%d (%d px de alto), lienzo %dx%d en piezas de 16x16 (lleno: %d)" % (
        prefijo, escala, 31 * escala, LIENZO_AN, alto, (LIENZO_AN // 16) * (alto // 16)))
    for i, piezas in enumerate(tabla):
        nombre = ("andar %d" % i) if i < len(andar) else ("ataque %d" % (i - len(andar)))
        print("  %-9s %2d piezas = %3d tiles, %4d bytes" % (nombre, len(piezas), len(piezas) * 4, grupos[i][2]))
    print("  máximo: %d piezas; reserva por enemigo %d tiles (libres para enemigos: 256)" % (
        max(len(t) for t in tabla), max(g[2] for g in grupos) // BYTES_TILE))
    print("  tiras: andar %d bytes, ataque %d bytes" % (len(tiras[0]), len(tiras[1])))
    print("PREDICCIÓN (límite %d ranuras de 8 px y %d sprites por línea)" % (RANURAS_MAX, SPRITES_LINEA_MAX))
    predecir("andando: peor cuadro de andar", peor_cuadro(tabla[:len(andar)]), alto)
    predecir("atacando: peor cuadro de ataque", peor_cuadro(tabla[len(andar):]), alto)
    return colores


def informe_celda_64x96(origen):
    """Solo informe, sin archivos: el samurái ×3 RECORTADO a una celda de 64×96 con el
    cuerpo centrado, como saldría un humano pedido a Pixellab en esa celda (la espada
    del ataque se corta en el borde). Bytes exactos = piezas × 128 (4 tiles de 32 B):
    lo que subiría un DMA del tamaño justo, sin redondear a pares de filas."""
    andar = cuadros_de(os.path.join(origen, "WALK.png"), 8)
    ataque = cuadros_de(os.path.join(origen, "ATTACK 1.png"))
    _, indexados = indexar([al_lienzo(c, 3, 96, 64, 32) for c in andar + ataque])
    tabla = [piezas_no_vacias(c) for c in indexados]
    print("\nninja3 en celda de 64x96 (×3 recortado al cuerpo; solo informe, sin archivos)")
    for i, piezas in enumerate(tabla):
        nombre = ("andar %d" % i) if i < len(andar) else ("ataque %d" % (i - len(andar)))
        print("  %-9s %2d piezas = %4d bytes exactos" % (nombre, len(piezas), len(piezas) * 128))
    maximo = max(len(t) for t in tabla)
    print("  máximo: %d piezas = %d tiles por hueco: caben %d huecos en los 256 tiles libres" % (
        maximo, maximo * 4, 256 // (maximo * 4)))
    print("PREDICCIÓN (límite %d ranuras de 8 px y %d sprites por línea)" % (RANURAS_MAX, SPRITES_LINEA_MAX))
    predecir("andando: peor cuadro de andar", peor_cuadro(tabla[:len(andar)]), 96)
    predecir("atacando: peor cuadro de ataque", peor_cuadro(tabla[len(andar):]), 96)


def main(argv=None):
    p = argparse.ArgumentParser(description="Ninjas provisionales y predicciones de la prueba de carga.")
    p.add_argument("--origen", default=ORIGEN)
    p.add_argument("--destino", default=DESTINO)
    a = p.parse_args(argv)
    os.makedirs(a.destino, exist_ok=True)

    colores = None
    with open(os.path.join(a.destino, "ninja_datos.h"), "w", encoding="utf-8", newline="\n") as f:
        f.write("/* ninja_datos.h — GENERADO por scripts/preparar_prueba_carga.py; no editar.\n"
                "   Tablas del ninja provisional de la ROM de prueba de carga (src/pantallas/prueba_carga.c),\n"
                "   en dos escalas (ninja3: 93 px de alto; ninja2: 62 px). Por cuadro: cuántas piezas de\n"
                "   16x16 tiene, en qué tira está (0 andar, 1 ataque), el desplazamiento de sus tiles en\n"
                "   la tira y los bytes que sube (pares de filas de 1 KB); y las piezas (dx, dy) dentro\n"
                "   del lienzo, en el orden de la VRAM (la pieza k en el tile k/8*32 + k%%8*2). */\n"
                "#ifndef NINJA_DATOS_H\n#define NINJA_DATOS_H\n\n"
                "#define NINJA_CENTRO_X %d          /* x del cuerpo dentro del lienzo.             */\n\n" % CENTRO_X)
        for escala, alto in ESCALAS.items():
            c = generar(a.origen, a.destino, escala, alto, f)
            colores = colores or c
        f.write("#endif /* NINJA_DATOS_H */\n")

    with open(os.path.join(a.destino, "ninja_pal.bin"), "wb") as f:
        f.write(paleta_bytes(colores))
        f.write(paleta_bytes(variante(colores, 0.33)))
        f.write(paleta_bytes(variante(colores, 0.66)))
        f.write(b"\x00\x00" * 16)                        # Paleta 4: la sombra (color 1 negro).
    with open(os.path.join(a.destino, "sombra.bin"), "wb") as f:
        f.write(sombra())
    informe_celda_64x96(a.origen)


if __name__ == "__main__":
    main()
