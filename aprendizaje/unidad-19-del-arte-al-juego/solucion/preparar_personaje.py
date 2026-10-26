#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    preparar_personaje.py — Convierte las animaciones de un personaje (exportadas
    de Pixellab) en las tiras indexadas que gfx4snes necesita, con UNA paleta
    compartida, y genera el header con la cantidad de cuadros de cada una.

    PROPÓSITO
      El arte llega en el formato que dé Pixellab: su exportación NATIVA
      (Characters → Export: <Estado>/animations/<nombre>/<dirección>/frame_NNN.png,
      8 direcciones, lienzos de 76..104 px) o, a mano, una tira PNG, una carpeta
      de cuadros o un GIF por animación. La SNES exige celdas de 96x64 (3x2
      piezas de 32x32), 15 colores + transparente, una sola paleta para todo el
      personaje y los pies siempre en la misma fila. Este script hace esa
      traducción SIN escalar el arte y, si falta una animación, fabrica un
      marcador numerado para que el juego compile sin esperar al arte.
      La exportación nativa trae otros nombres y otros cortes que las 13
      animaciones del juego: el diccionario MAPEOS_PIXELLAB dice de qué estado,
      dirección y tramo de cuadros sale cada una (y si va al revés).

    SE CONECTA CON
      Lee:      res/descargas/graficos/pixellab/<nombre>/ (ver su README.md): la
                exportación nativa de Pixellab o un archivo por animación.
      Escribe:  res/personajes/<nombre>/<nombre>_<animación>.png (13 tiras),
                <nombre>_reposo_estatico.png (SIEMPRE 2 cuadros del reposo, para
                la carga estática de intro y título en src/pantallas/personajes.c)
                y res/personajes/<nombre>/<nombre>_anim.h (#define *_CUADROS).
      Lo usan:  el Makefile (gfx4snes -s 32 -T convierte las tiras),
                src/juego/animaciones.c (incluye <nombre>_anim.h) y
                scripts/pruebas/prueba_preparar_personaje.py (pruebas).

    HARDWARE QUE TOCA
      ninguno; produce datos para la ROM. Las reglas que aplica son del hardware:
      índice 0 = transparente en sprites, 16 colores por paleta, y cada tira en
      su propia sección de 32 KB (de ahí el máximo de 10 cuadros de 3 KB).

    VARIABLES (constantes del módulo)
      CELDA_ANCHO   int    96 px             ancho de la celda (3 piezas de 32)
      CELDA_ALTO    int    64 px             alto de la celda (2 piezas de 32); la última fila son los pies
      MAX_CUADROS   int    10                cuadros por animación (10 x 3 KB = 30 KB < 32 KB)
      CUADROS_ESTATICOS int 2                cuadros de la tira estática de reposo (6 KB en VRAM)
      COLORES       int    15                colores opacos (el 16.º es el transparente)
      TRANSPARENTE  tuple  magenta           color que marca el índice 0 al guardar
      ANIMACIONES   list   (nombre, cuadros) las 13 animaciones y sus cuadros por defecto
      MAPEOS_PIXELLAB dict personaje -> {animación: (estado, dirección, desde, hasta, al_revés)
                                         o una lista de esas reglas, por orden de preferencia}

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

CELDA_ANCHO = 96
CELDA_ALTO = 64
MAX_CUADROS = 10
CUADROS_ESTATICOS = 2
COLORES = 15
TRANSPARENTE = (255, 0, 255)

# Las 13 animaciones de la Revisión 1 (docs/diseno-demo-jugable.md §7), con los
# cuadros que tiene el marcador cuando el arte real todavía no está.
ANIMACIONES = [
    ("reposo", 4), ("caminar", 8), ("correr", 8), ("saltar", 2), ("caer", 2),
    ("agachado", 2), ("agachado_caminar", 6), ("desenvainar", 4), ("envainar", 4),
    ("atacar", 5), ("echarse", 4), ("dormido", 4), ("despertar", 4),
]

# De qué parte de la exportación NATIVA de Pixellab sale cada animación del juego
# (decisión del 2026-10-19, docs/diseno-demo-jugable.md §10). Cada entrada es
# (estado de Pixellab, dirección, primer cuadro, cuadro final sin incluir o None
# = hasta el último, al revés). primer cuadro = None significa una pose FIJA: la
# imagen <estado>/rotations/<dirección>.png como único cuadro. Todo mira al este
# (la izquierda la hace el espejo del hardware) salvo dormir: Pixellab solo
# generó Sleep mirando al sur.
MAPEOS_PIXELLAB = {
    "tarou": {
        "reposo": ("Idle", "east", 0, None, False),
        "caminar": ("Walk", "east", 0, None, False),
        "correr": ("Run", "east", 0, None, False),
        "saltar": ("Jump", "east", 2, 5, False),           # impulso y subida
        "caer": ("Jump", "east", 5, 8, False),             # bajada (el 8.º es el aterrizaje)
        "agachado": ("Sneak", "east", 0, 1, False),        # un cuadro de Sneak, quieto
        "agachado_caminar": ("Sneak", "east", 0, None, False),
        "desenvainar": ("Unsheathe", "east", 0, None, False),
        "envainar": ("Unsheathe", "east", 0, None, True),  # el mismo gesto, al revés
        # Una LISTA de reglas: se usa la primera que exista en la exportación. Cuando
        # Pixellab genere la animación «Attack», entra sola; mientras, la pose fija.
        "atacar": [("Attack", "east", 0, None, False), ("Attack_1", "east", None, None, False)],
        "echarse": ("Stand_up", "east", 0, 6, True),       # levantarse al revés = tumbarse
        "dormido": ("Sleep", "south", 0, None, False),
        "despertar": ("Stand_up", "east", 0, 10, False),   # 11 cuadros no caben: 10
    },
}


# --- 1. Cargar los cuadros de una animación en el formato que haya -----------------

def cargar_cuadros(entrada: Path, animacion: str) -> list | None:
    """Devuelve la lista de cuadros RGBA de `animacion`, o None si no hay arte."""
    tira = entrada / f"{animacion}.png"
    carpeta = entrada / animacion
    gif = entrada / f"{animacion}.gif"
    if tira.is_file():
        with Image.open(tira) as im:
            im = im.convert("RGBA")
            lado = im.height                                       # cuadros cuadrados
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


# --- 1b. Exportación nativa de Pixellab --------------------------------------------

def es_pixellab_nativo(entrada: Path) -> bool:
    """La exportación nativa tiene una carpeta por estado con animations/ o rotations/."""
    return entrada.is_dir() and any((d / "animations").is_dir() or (d / "rotations").is_dir()
                                   for d in entrada.iterdir() if d.is_dir())


def anclar(cuadros: list) -> list:
    """Coloca cada cuadro de UNA animación en una celda de CELDA_ANCHO x CELDA_ALTO sin
    escalar: el centro del lienzo de Pixellab va a la columna central de la celda y
    la fila más baja con píxeles de toda la animación (los pies) a la última fila.
    Los lienzos miden distinto según el estado (76..104 px), así que se ancla por
    el centro y los pies, nunca por la esquina."""
    pies = max(c.getchannel("A").getbbox()[3] for c in cuadros if c.getchannel("A").getbbox())
    celdas = []
    for c in cuadros:
        celda = Image.new("RGBA", (CELDA_ANCHO, CELDA_ALTO), (0, 0, 0, 0))
        x = (CELDA_ANCHO - c.width) // 2              # centro del lienzo -> centro de la celda
        y = CELDA_ALTO - pies                         # pies -> última fila
        celda.paste(c, (x, y), c)                     # lo que quede fuera se recorta
        b = c.getchannel("A").getbbox()
        if b and (b[0] + x < 0 or b[2] + x > CELDA_ANCHO or b[1] + y < 0):
            print(f"AVISO: un cuadro no cabe en {CELDA_ANCHO}x{CELDA_ALTO} y se recorta")
        celdas.append(celda)
    return celdas


def cargar_pixellab(entrada: Path, regla: tuple) -> list | None:
    """Cuadros de una animación del juego según su regla de MAPEOS_PIXELLAB, ya
    anclados en celdas. None si ese estado o dirección no está en la exportación."""
    estado, direccion, desde, hasta, al_reves = regla
    if desde is None:                                 # pose fija: rotations/<dirección>.png
        pose = entrada / estado / "rotations" / f"{direccion}.png"
        if not pose.is_file():
            return None
        with Image.open(pose) as im:
            return anclar([im.convert("RGBA")])
    carpetas = sorted((entrada / estado / "animations").glob(f"*/{direccion}"))
    if not carpetas:
        return None
    if len(carpetas) > 1:
        print(f"AVISO: {estado} tiene {len(carpetas)} animaciones; uso {carpetas[0].parent.name}")
    archivos = sorted(carpetas[0].glob("*.png"))
    cuadros = []
    for f in archivos:
        with Image.open(f) as im:
            cuadros.append(im.convert("RGBA"))
    cuadros = cuadros[desde:hasta]
    if al_reves:
        cuadros = cuadros[::-1]
    return anclar(cuadros) if cuadros else None


# --- 2. Marcador numerado para las animaciones que faltan --------------------------

def marcador(animacion: str, k: int, total: int) -> Image.Image:
    """Celda de 96x64 con un marco, el nombre de la animación y 'k/total'."""
    im = Image.new("RGBA", (CELDA_ANCHO, CELDA_ALTO), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle((20, 4, CELDA_ANCHO - 21, CELDA_ALTO - 1), fill=(70, 70, 150, 255), outline=(240, 240, 240, 255), width=2)
    fuente = ImageFont.load_default(size=9)
    lineas = animacion.split("_") + [f"{k + 1}/{total}"]
    for i, linea in enumerate(lineas):
        d.text((24, 10 + i * 13), linea, fill=(240, 240, 240, 255), font=fuente)
    return im


# --- 3. Normalizar cada cuadro a la celda de 96x64 ----------------------------------

def a_celda(cuadro: Image.Image) -> Image.Image:
    """Mete un cuadro en una celda de 96x64: centrado y apoyado abajo si cabe,
    reducido (con pérdida, y avisando) solo si no cabe."""
    if cuadro.size == (CELDA_ANCHO, CELDA_ALTO):
        return cuadro
    if cuadro.width > CELDA_ANCHO or cuadro.height > CELDA_ALTO:
        factor = min(CELDA_ANCHO / cuadro.width, CELDA_ALTO / cuadro.height)
        cuadro = cuadro.resize((max(1, round(cuadro.width * factor)), max(1, round(cuadro.height * factor))), Image.NEAREST)
        print(f"AVISO: cuadro mayor que {CELDA_ANCHO}x{CELDA_ALTO}, reducido a {cuadro.size} (pierde detalle)")
    celda = Image.new("RGBA", (CELDA_ANCHO, CELDA_ALTO), (0, 0, 0, 0))
    celda.paste(cuadro, ((CELDA_ANCHO - cuadro.width) // 2, CELDA_ALTO - cuadro.height), cuadro)
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
        f"      {N}_<ANIMACION>_CUADROS  1..{MAX_CUADROS}  cuadros de 96x64 de esa tira",
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
    nativo = es_pixellab_nativo(entrada)
    mapeo = MAPEOS_PIXELLAB.get(nombre, {})
    if nativo and not mapeo:
        sys.exit(f"ERROR: {entrada} es una exportación nativa de Pixellab pero no hay mapeo para '{nombre}' "
                 f"en MAPEOS_PIXELLAB. No se escribió nada.")
    for animacion, por_defecto in ANIMACIONES:
        if nativo:
            reglas = mapeo.get(animacion, [])
            if isinstance(reglas, tuple):
                reglas = [reglas]                     # una sola regla
            cuadros = None
            for regla in reglas:                      # la primera que esté en la exportación
                cuadros = cargar_pixellab(entrada, regla)
                if cuadros is not None:
                    break
            origen = "pixellab"
        else:
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
    # La exportación nativa de Pixellab tampoco: cargar_pixellab ya la ancló por
    # el centro de su lienzo y los pies, que es el registro que el arte trae.
    arte = [c for cuadros, origen in por_animacion.values() if origen == "arte" for c in cuadros]
    cajas = [c.getbbox() for c in arte if c.getbbox()]
    if cajas:
        L, T = min(b[0] for b in cajas), min(b[1] for b in cajas)
        R, B = max(b[2] for b in cajas), max(b[3] for b in cajas)
        dx = (CELDA_ANCHO - (R - L)) // 2             # centrado horizontal
        dy = CELDA_ALTO - (B - T)                     # pies en la última fila
        medida = f"personaje {R - L}x{B - T} px"
    else:
        L = T = 0; R = CELDA_ANCHO; B = CELDA_ALTO; dx = dy = 0   # nada que alinear
        medida = "exportación nativa de Pixellab (anclada por centro y pies)" if nativo else "solo marcadores"
    def alinear(c):
        celda = Image.new("RGBA", (CELDA_ANCHO, CELDA_ALTO), (0, 0, 0, 0))
        recorte = c.crop((L, T, R, B))
        celda.paste(recorte, (dx, dy), recorte)
        return celda
    alineados = {a: ([alinear(c) for c in cuadros] if origen == "arte" else cuadros)
                 for a, (cuadros, origen) in por_animacion.items()}

    colores = paleta_comun([c for cuadros in alineados.values() for c in cuadros])

    salida.mkdir(parents=True, exist_ok=True)
    resultado = {}
    for animacion, cuadros in alineados.items():
        tira = Image.new("RGBA", (CELDA_ANCHO * len(cuadros), CELDA_ALTO), (0, 0, 0, 0))
        for k, c in enumerate(cuadros):
            tira.paste(c, (k * CELDA_ANCHO, 0), c)
        indexar(tira, colores).save(salida / f"{nombre}_{animacion}.png", format="PNG")
        resultado[animacion] = len(cuadros)
        print(f"  {nombre}_{animacion}.png  {len(cuadros)} cuadros  ({por_animacion[animacion][1]})")
    # Tira ESTÁTICA de reposo para intro y título (src/pantallas/personajes.c):
    # allí los cuadros se cargan de una vez en VRAM (2 x 3 KB) sin motor dinámico,
    # junto a Bárbara, y en 512 tiles de sprites no caben más: SIEMPRE 2, tenga
    # los que tenga el reposo real. Se eligen repartidos a lo largo del reposo
    # (el k-ésimo es el (k*N)//2): con 6 cuadros, el 0 y el 3; con 9, el 0 y el 4.
    reposo = alineados["reposo"]
    estaticos = [reposo[(k * len(reposo)) // CUADROS_ESTATICOS] for k in range(CUADROS_ESTATICOS)]
    tira = Image.new("RGBA", (CELDA_ANCHO * CUADROS_ESTATICOS, CELDA_ALTO), (0, 0, 0, 0))
    for k, c in enumerate(estaticos):
        tira.paste(c, (k * CELDA_ANCHO, 0), c)
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
