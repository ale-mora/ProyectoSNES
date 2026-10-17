#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    preparar_personaje.py — Convierte las animaciones de un personaje (exportadas
    de Pixellab) en las tiras indexadas que gfx4snes necesita, con UNA paleta
    compartida, y genera el header con la cantidad de cuadros de cada una.

    PROPÓSITO
      El arte llega en el formato que dé Pixellab (tira PNG, carpeta de cuadros o
      GIF); la SNES exige cuadros de 64x64, 15 colores + transparente, una sola
      paleta para todo el personaje y los pies siempre en la misma fila. Este
      script hace esa traducción y, si falta una animación, fabrica un marcador
      numerado para que el juego compile y se pueda probar sin esperar al arte.

    SE CONECTA CON
      Lee:      res/descargas/graficos/pixellab/<nombre>/ (ver su README.md).
      Escribe:  res/personajes/<nombre>/<nombre>_<animación>.png (13 tiras),
                <nombre>_reposo_estatico.png (SIEMPRE 4 cuadros del reposo, para
                la carga estática de intro y título en src/pantallas/personajes.c)
                y res/personajes/<nombre>/<nombre>_anim.h (#define *_CUADROS).
      Lo usan:  el Makefile (gfx4snes -s 32 -T convierte las tiras),
                src/juego/animaciones.c (incluye <nombre>_anim.h) y
                scripts/pruebas/prueba_preparar_personaje.py (pruebas).

    HARDWARE QUE TOCA
      ninguno; produce datos para la ROM. Las reglas que aplica son del hardware:
      índice 0 = transparente en sprites, 16 colores por paleta, y cada tira en
      su propia sección de 32 KB (de ahí el máximo de 15 cuadros de 2 KB).

    VARIABLES (constantes del módulo)
      CELDA         int    64 px             lado del cuadro
      MAX_CUADROS   int    15                cuadros por animación (30 KB < 32 KB)
      CUADROS_ESTATICOS int 4                cuadros de la tira estática de reposo (8 KB en VRAM)
      COLORES       int    15                colores opacos (el 16.º es el transparente)
      TRANSPARENTE  tuple  magenta           color que marca el índice 0 al guardar
      ANIMACIONES   list   (nombre, cuadros) las 13 animaciones y sus cuadros por defecto

    Uso (desde la raíz del repo):
      python scripts/preparar_personaje.py tarou
      python scripts/preparar_personaje.py barbara --entrada RUTA --salida RUTA
    → glosario: Sprite, Paleta, Banco / LoROM, Metasprite
---------------------------------------------------------------------------------
"""

import argparse
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont, ImageSequence

CELDA = 64
MAX_CUADROS = 15
CUADROS_ESTATICOS = 4
COLORES = 15
TRANSPARENTE = (255, 0, 255)

# Las 13 animaciones de la Revisión 1 (docs/diseno-demo-jugable.md §7), con los
# cuadros que tiene el marcador cuando el arte real todavía no está.
ANIMACIONES = [
    ("reposo", 4), ("caminar", 8), ("correr", 8), ("saltar", 2), ("caer", 2),
    ("agachado", 2), ("agachado_caminar", 6), ("desenvainar", 4), ("envainar", 4),
    ("atacar", 5), ("echarse", 4), ("dormido", 4), ("despertar", 4),
]


# --- 1. Cargar los cuadros de una animación en el formato que haya -----------------

def cargar_cuadros(entrada: Path, animacion: str) -> list | None:
    """Devuelve la lista de cuadros RGBA de `animacion`, o None si no hay arte."""
    tira = entrada / f"{animacion}.png"
    carpeta = entrada / animacion
    gif = entrada / f"{animacion}.gif"
    if tira.is_file():
        with Image.open(tira) as im:
            im = im.convert("RGBA")
            lado = CELDA if im.height == CELDA else im.height      # cuadros cuadrados
            if im.width % lado != 0:
                sys.exit(f"ERROR: {tira} mide {im.width}x{im.height}: el ancho no es múltiplo de {lado}")
            return [im.crop((k * lado, 0, (k + 1) * lado, im.height)) for k in range(im.width // lado)]
    if carpeta.is_dir():
        archivos = sorted(p for p in carpeta.iterdir() if p.suffix.lower() == ".png")
        if archivos:
            cuadros = []
            for p in archivos:
                with Image.open(p) as im:
                    cuadros.append(im.convert("RGBA"))
            return cuadros
    if gif.is_file():
        with Image.open(gif) as im:
            return [cuadro.convert("RGBA") for cuadro in ImageSequence.Iterator(im)]
    return None


# --- 2. Marcador numerado para las animaciones que faltan --------------------------

def marcador(animacion: str, k: int, total: int) -> Image.Image:
    """Cuadro de 64x64 con un marco, el nombre de la animación y 'k/total'."""
    im = Image.new("RGBA", (CELDA, CELDA), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle((4, 4, CELDA - 5, CELDA - 1), fill=(70, 70, 150, 255), outline=(240, 240, 240, 255), width=2)
    fuente = ImageFont.load_default(size=9)
    lineas = animacion.split("_") + [f"{k + 1}/{total}"]
    for i, linea in enumerate(lineas):
        d.text((8, 10 + i * 13), linea, fill=(240, 240, 240, 255), font=fuente)
    return im


# --- 3. Normalizar cada cuadro a 64x64 ---------------------------------------------

def a_celda(cuadro: Image.Image) -> Image.Image:
    """Mete un cuadro en una celda de 64x64: centrado y apoyado abajo si es menor,
    reducido (con pérdida) si es mayor."""
    if cuadro.size == (CELDA, CELDA):
        return cuadro
    if cuadro.width > CELDA or cuadro.height > CELDA:
        factor = CELDA / max(cuadro.width, cuadro.height)
        cuadro = cuadro.resize((max(1, round(cuadro.width * factor)), max(1, round(cuadro.height * factor))), Image.NEAREST)
        print(f"AVISO: cuadro mayor que {CELDA}x{CELDA}, reducido a {cuadro.size} (pierde detalle)")
    celda = Image.new("RGBA", (CELDA, CELDA), (0, 0, 0, 0))
    celda.paste(cuadro, ((CELDA - cuadro.width) // 2, CELDA - cuadro.height), cuadro)
    return celda


# --- 4. Paleta común y tiras indexadas ---------------------------------------------

def paleta_comun(cuadros: list) -> list:
    """Hasta 15 colores opacos para TODOS los cuadros: exactos si ya son ≤ 15,
    cuantizados (median cut) si hay más."""
    # Pillow ≥ 12 llama get_flattened_data a lo que antes era getdata.
    datos = (lambda im: im.get_flattened_data() if hasattr(im, "get_flattened_data") else im.getdata())
    pixeles = [p[:3] for im in cuadros for p in datos(im) if p[3] > 0]
    unicos = sorted(set(pixeles))
    if len(unicos) <= COLORES:
        return unicos
    fila = Image.new("RGB", (len(pixeles), 1))
    fila.putdata(pixeles)
    q = fila.quantize(colors=COLORES, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    pal = q.getpalette()[: COLORES * 3]
    colores = [tuple(pal[i * 3: i * 3 + 3]) for i in range(COLORES)]
    return sorted(set(colores))


def indexar(tira_rgba: Image.Image, colores: list) -> Image.Image:
    """Tira en modo P: índice 0 = transparente (magenta en la paleta), 1.. = colores."""
    paleta = list(TRANSPARENTE) + [c for col in colores for c in col]
    paleta += [0, 0, 0] * (256 - 1 - len(colores))
    salida = Image.new("P", tira_rgba.size, 0)
    salida.putpalette(paleta)
    cache = {}
    px_src = tira_rgba.load()
    px_dst = salida.load()
    for y in range(tira_rgba.height):
        for x in range(tira_rgba.width):
            r, g, b, a = px_src[x, y]
            if a == 0:
                continue                                   # queda en 0 (transparente)
            clave = (r, g, b)
            if clave not in cache:                         # color más cercano de la paleta
                cache[clave] = 1 + min(range(len(colores)),
                                       key=lambda i: (colores[i][0] - r) ** 2 + (colores[i][1] - g) ** 2 + (colores[i][2] - b) ** 2)
            px_dst[x, y] = cache[clave]
    return salida


# --- 5. Header con la cantidad de cuadros --------------------------------------------

def escribir_header(ruta: Path, nombre: str, cuadros: dict) -> None:
    N = nombre.upper()
    lineas = [
        "/*---------------------------------------------------------------------------------",
        f"    {nombre}_anim.h — Cuántos cuadros tiene cada animación de {nombre}.",
        "",
        "    PROPÓSITO",
        "      Lo GENERA scripts/preparar_personaje.py a partir del arte (o de los",
        "      marcadores): no editar a mano. Así el código no necesita saber cuántos",
        "      cuadros dibujó el artista: la tabla de animaciones lo lee de aquí.",
        "",
        "    SE CONECTA CON",
        f"      Lo usan: src/juego/animaciones.c (tabla ANIMACIONES) y tests/ (copia fija).",
        "",
        "    HARDWARE QUE TOCA",
        "      ninguno.",
        "",
        "    VARIABLES (constantes)",
        f"      {N}_<ANIMACION>_CUADROS  1..{MAX_CUADROS}  cuadros de 64x64 de esa tira",
        f"      {N}_ANIMACIONES          {len(cuadros)}      cuántas animaciones hay",
        "---------------------------------------------------------------------------------*/",
        f"#ifndef {N}_ANIM_H",
        f"#define {N}_ANIM_H",
        "",
    ]
    for animacion, n in cuadros.items():
        lineas.append(f"#define {N}_{animacion.upper()}_CUADROS {n}")
    lineas += ["", f"#define {N}_ANIMACIONES {len(cuadros)}", "", f"#endif /* {N}_ANIM_H */", ""]
    ruta.write_text("\n".join(lineas), encoding="utf-8")


# --- 6. Todo junto ------------------------------------------------------------------

def preparar(nombre: str, entrada: Path, salida: Path) -> dict:
    """Prepara las 13 animaciones de `nombre`. Devuelve {animación: cuadros}.
    No escribe nada si alguna animación no cumple el límite de cuadros."""
    por_animacion = {}
    for animacion, por_defecto in ANIMACIONES:
        cuadros = cargar_cuadros(entrada, animacion)
        origen = "arte"
        if cuadros is None:
            cuadros = [marcador(animacion, k, por_defecto) for k in range(por_defecto)]
            origen = "marcador"
        if len(cuadros) > MAX_CUADROS:
            sys.exit(f"ERROR: {animacion} tiene {len(cuadros)} cuadros y el máximo es {MAX_CUADROS} "
                     f"(cada animación debe caber en un banco de 32 KB). No se escribió nada.")
        por_animacion[animacion] = ([a_celda(c) for c in cuadros], origen)

    # Caja envolvente común a TODAS las animaciones con ARTE: así ninguna "salta"
    # respecto a otra (reposo y agachado conservan su altura relativa). Los
    # marcadores no cuentan: ocupan la celda entera y anularían el desplazamiento.
    arte = [c for cuadros, origen in por_animacion.values() if origen == "arte" for c in cuadros]
    cajas = [c.getbbox() for c in arte if c.getbbox()]
    if cajas:
        L, T = min(b[0] for b in cajas), min(b[1] for b in cajas)
        R, B = max(b[2] for b in cajas), max(b[3] for b in cajas)
        dx = (CELDA - (R - L)) // 2                   # centrado horizontal
        dy = CELDA - (B - T)                          # pies en la última fila
        medida = f"personaje {R - L}x{B - T} px"
    else:
        L = T = 0; R = B = CELDA; dx = dy = 0         # solo marcadores: nada que alinear
        medida = "solo marcadores"
    def alinear(c):
        celda = Image.new("RGBA", (CELDA, CELDA), (0, 0, 0, 0))
        recorte = c.crop((L, T, R, B))
        celda.paste(recorte, (dx, dy), recorte)
        return celda
    alineados = {a: ([alinear(c) for c in cuadros] if origen == "arte" else cuadros)
                 for a, (cuadros, origen) in por_animacion.items()}

    colores = paleta_comun([c for cuadros in alineados.values() for c in cuadros])

    salida.mkdir(parents=True, exist_ok=True)
    resultado = {}
    for animacion, cuadros in alineados.items():
        tira = Image.new("RGBA", (CELDA * len(cuadros), CELDA), (0, 0, 0, 0))
        for k, c in enumerate(cuadros):
            tira.paste(c, (k * CELDA, 0), c)
        indexar(tira, colores).save(salida / f"{nombre}_{animacion}.png", format="PNG")
        resultado[animacion] = len(cuadros)
        print(f"  {nombre}_{animacion}.png  {len(cuadros)} cuadros  ({por_animacion[animacion][1]})")
    # Tira ESTÁTICA de reposo para intro y título (src/pantallas/personajes.c):
    # allí los cuadros se cargan de una vez en VRAM (4 x 2 KB) sin motor dinámico,
    # así que tienen que ser SIEMPRE 4, tenga los que tenga el reposo real. Se
    # eligen repartidos a lo largo del reposo (el k-ésimo es el (k*N)//4) para que
    # el bucle no salte: con 6 cuadros, el 0, 1, 3 y 4; con 2, el 0, 0, 1 y 1.
    reposo = alineados["reposo"]
    estaticos = [reposo[(k * len(reposo)) // CUADROS_ESTATICOS] for k in range(CUADROS_ESTATICOS)]
    tira = Image.new("RGBA", (CELDA * CUADROS_ESTATICOS, CELDA), (0, 0, 0, 0))
    for k, c in enumerate(estaticos):
        tira.paste(c, (k * CELDA, 0), c)
    indexar(tira, colores).save(salida / f"{nombre}_reposo_estatico.png", format="PNG")
    print(f"  {nombre}_reposo_estatico.png  {CUADROS_ESTATICOS} cuadros  (reposo {[(k * len(reposo)) // CUADROS_ESTATICOS for k in range(CUADROS_ESTATICOS)]})")
    escribir_header(salida / f"{nombre}_anim.h", nombre, resultado)
    print(f"  {nombre}_anim.h  paleta: {len(colores)} colores + transparente; {medida}")
    return resultado


def main() -> int:
    ap = argparse.ArgumentParser(description="Prepara las animaciones de un personaje para gfx4snes.")
    ap.add_argument("nombre", help="personaje: tarou, barbara...")
    ap.add_argument("--entrada", type=Path, default=None, help="carpeta de Pixellab (por defecto res/descargas/graficos/pixellab/<nombre>)")
    ap.add_argument("--salida", type=Path, default=None, help="carpeta de salida (por defecto res/personajes/<nombre>)")
    args = ap.parse_args()
    entrada = args.entrada or Path("res/descargas/graficos/pixellab") / args.nombre
    salida = args.salida or Path("res/personajes") / args.nombre
    print(f"[{args.nombre}] entrada: {entrada}  salida: {salida}")
    preparar(args.nombre, entrada, salida)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
