#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    preparar_personaje.py — Convierte las animaciones de un personaje (exportadas
    de Pixellab) en las tiras indexadas que gfx4snes necesita, con UNA paleta
    compartida, y genera el header con la cantidad de cuadros de cada una.

    PROPÓSITO
      El arte llega en el formato que dé Pixellab: su exportación NATIVA
      (Characters → Export: <Estado>/animations/<nombre>/<dirección>/frame_NNN.png,
      8 direcciones, lienzos de 76..144 px) o, a mano, una tira PNG, una carpeta
      de cuadros o un GIF por animación. La SNES exige una celda fija por
      personaje (96x64 para Tarou; 64x96 para Bárbara y los ninjas del beat
      'em up), 15 colores + transparente, una sola paleta para todo el
      personaje y los pies siempre en la misma fila. Este script hace esa
      traducción SIN escalar el arte y, si falta una animación, fabrica un
      marcador numerado para que el juego compile sin esperar al arte.
      La exportación nativa trae otros nombres y otros cortes que las
      animaciones del juego: el diccionario MAPEOS_PIXELLAB dice de qué estado,
      dirección y tramo de cuadros sale cada una (y si va al revés).
      PERSONAJES da, por personaje, la celda, la lista de animaciones, si lleva
      tira estática de reposo y los COLORES FIJOS: tonos que la paleta guarda
      siempre (piel, acero) porque, si no, el color que ocupa más superficie (el
      traje) se queda con los 15 y los detalles pequeños se pierden.

    SE CONECTA CON
      Lee:      res/descargas/graficos/pixellab/<nombre>/ (ver su README.md): la
                exportación nativa de Pixellab o un archivo por animación.
      Escribe:  res/personajes/<nombre>/<nombre>_<animación>.png (14 tiras),
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
      CELDA_ANCHO   int    96 px             ancho de la celda por defecto (la de Tarou)
      CELDA_ALTO    int    64 px             alto de la celda por defecto; la última fila son los pies
      MAX_CUADROS   int    10                cuadros por animación (10 x 3 KB = 30 KB < 32 KB)
      CUADROS_ESTATICOS int 2                cuadros de la tira estática de reposo (6 KB en VRAM)
      COLORES       int    15                colores opacos (el 16.º es el transparente)
      TOLERANCIA_FIJOS int 32                distancia RGB a un color fijo por debajo de la cual
                                             un píxel ya está "cubierto" y no cuenta para el resto
      UNIFICAR_DISTANCIA int 8               colores de median cut más cercanos que esto se unifican
      UNIFICAR_INTENTOS int 12               tamaños de median cut que se prueban para llenar los huecos
      TRANSPARENTE  tuple  magenta           color que marca el índice 0 al guardar
      ANIMACIONES   list   (nombre, cuadros) las 14 animaciones de Tarou y sus cuadros por defecto
      ANIMACIONES_BARBARA, ANIMACIONES_NINJA las de la Revisión 3 (docs/pixellab-fase1.md §1.2, §2.2)
      MAPEOS_PIXELLAB dict personaje -> {animación: (estado, dirección, desde, hasta, al_revés)}
      PERSONAJES    dict personaje -> {celda, animaciones, estatico, fijos}

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
TOLERANCIA_FIJOS = 32
UNIFICAR_DISTANCIA = 8      # dos colores a esta distancia RGB o menos son "el mismo"
UNIFICAR_INTENTOS = 12      # cuántos k de median cut se prueban para llenar los huecos
TRANSPARENTE = (255, 0, 255)

# Las 13 animaciones de la Revisión 1 (docs/diseno-demo-jugable.md §7) más la pose
# de guardia (unidad 18 del curso), con los cuadros que tiene el marcador cuando el
# arte real todavía no está.
ANIMACIONES = [
    ("reposo", 4), ("caminar", 8), ("correr", 8), ("saltar", 2), ("caer", 2),
    ("agachado", 2), ("agachado_caminar", 6), ("desenvainar", 4), ("envainar", 4),
    ("atacar", 5), ("echarse", 4), ("dormido", 4), ("despertar", 4),
    ("guardia", 1),     # Tarou quieto con la espada fuera (un dibujo, no un estado)
]

# De qué parte de la exportación NATIVA de Pixellab sale cada animación del juego
# (decisión del 2026-10-19, docs/diseno-demo-jugable.md §10). Cada entrada es
# (estado de Pixellab [o "estado/animación" si el estado tiene varias], dirección, primer cuadro, cuadro final sin incluir o None
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
        "atacar": ("Attack_1", "east", None, None, False), # PROVISIONAL: pose fija
        "echarse": ("Stand_up", "east", 0, 6, True),       # levantarse al revés = tumbarse
        "dormido": ("Sleep", "south", 0, None, False),
        "despertar": ("Stand_up", "east", 0, 10, False),   # 11 cuadros no caben: 10
        "guardia": ("Fight_Pose", "east", None, None, False),  # pose fija: espada en la boca
    },
    # PROVISIONAL (2026-11-04): solo lo que ya trae la primera exportación; el
    # resto sale como marcador. Los tramos se ajustarán al revisar el arte.
    "barbara": {
        "guardia": ("Fighting_pose_with_a", "east", 0, None, False),  # guardia con katana (8)
        "caminar": ("Walking", "east", 0, None, False),                # 6 (se pidieron 8)
        "golpe_3": ("Attack_1", "east", 0, None, False),               # tajo descendente (9)
    },
    "ninja": {
        "guardia": ("Fighting_stance/Fight_Stance_Idle", "east", 0, None, False),  # 8 (se pidieron 4)
        # Generadas el 2026-11-04 con el MCP de Pixellab (skeleton-v3, solo el este)
        # sobre el estado Fighting_stance. El cuadro 0 es la pose de partida.
        "bloqueo": ("Fighting_stance/bloqueo", "east", 1, 3, False),     # brazos arriba (2)
        # agarre y lanzamiento, "en el sitio": sin avanzar, para que quepan en 64 px
        # (el juego mueve al ninja); las versiones que avanzaban se salían 10-12 px.
        "agarre": ("Fighting_stance/agarre_sitio", "east", 1, 3, False),     # estira y cierra las manos (2)
        "lanzamiento": ("Fighting_stance/lanzamiento_corto", "east", 0, None, False),  # encoge y empuja (6)
        "agarrado": ("Fighting_stance/agarrado", "east", 1, 3, False),   # forcejea agachado (2)
        "caminar": ("Walking", "east", 0, None, False),                # 6 (se pidieron 8)
        "ataque_1": ("Jab_Punch", "east", 0, None, False),             # puñetazo, sin espada
        "ataque_2": ("Throwing_shuriken", "east", 0, None, False),     # 9
        "recibir": ("Taking_Punch", "east", 0, None, False),           # 6 (se pidieron 2)
        "derribado": ("Falling_back", "east", 0, None, False),         # 7
        "en_suelo": ("Defeated", "east", None, None, False),           # pose fija, tumbado
        "levantarse": ("Get_up", "east", 0, None, False),              # 5
        "ko": ("Falling_defeated", "east", 0, None, False),            # 7
    },
}

# Animaciones de la Revisión 3 con los cuadros del marcador (docs/pixellab-fase1.md).
ANIMACIONES_BARBARA = [
    ("guardia", 4), ("caminar", 8), ("correr", 8), ("saltar", 3), ("caer", 3),
    ("golpe_1", 5), ("golpe_2", 5), ("golpe_3", 7), ("aereo", 4), ("agarre", 2),
    ("golpe_agarre", 3), ("lanzamiento", 7), ("agarrada", 2), ("recibir", 2),
    ("derribada", 5), ("en_suelo", 1), ("levantarse", 6), ("especial", 8),
    ("lanzar_arma", 4), ("recoger", 2), ("entrada_relevo", 6), ("invocar", 4),
    ("ko", 6), ("victoria", 6), ("intro_venganza", 4),
]
ANIMACIONES_NINJA = [
    ("guardia", 4), ("caminar", 8), ("ataque_1", 4), ("ataque_2", 4), ("bloqueo", 2),
    ("agarre", 2), ("lanzamiento", 6), ("recibir", 2), ("derribado", 5),
    ("en_suelo", 1), ("levantarse", 5), ("agarrado", 2), ("ko", 4),
]

# Cada personaje: celda (ancho, alto), animaciones, si lleva la tira estática de
# reposo (solo Tarou: la usa src/pantallas/personajes.c) y los colores FIJOS.
# Los fijos van primero en la paleta (índices 1..n, siempre los mismos) y se
# muestrearon del arte real el 2026-11-04: sin ellos, el traje negro de Bárbara
# y el blanco del ninja se quedan los 15 colores, la katana sale color piel y
# la cara del ninja, gris.
PERSONAJES = {
    "tarou": {"celda": (96, 64), "animaciones": ANIMACIONES, "estatico": True, "fijos": []},
    "barbara": {
        "celda": (64, 96), "animaciones": ANIMACIONES_BARBARA, "estatico": False,
        "fijos": [
            (248, 204, 173), (209, 132, 111), (175, 102, 90),   # piel: luz, medio, sombra
            (83, 58, 46),                                       # pelo castaño
            (255, 255, 255), (182, 188, 189), (104, 106, 112),  # acero de la katana y placa del cinturón
        ],
    },
    "ninja": {
        "celda": (64, 96), "animaciones": ANIMACIONES_NINJA, "estatico": False,
        "fijos": [
            (244, 164, 107), (208, 112, 79), (176, 82, 68),     # piel alrededor de los ojos
            (68, 16, 13),                                       # línea oscura del ojo
        ],
    },
}
# Un nombre que no está en PERSONAJES se trata como Tarou (celda 96x64, 14 animaciones).
POR_DEFECTO = PERSONAJES["tarou"]


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


def anclar(cuadros: list, celda_wh: tuple = (CELDA_ANCHO, CELDA_ALTO), nombre: str = "") -> list:
    """Coloca cada cuadro de UNA animación en una celda de celda_wh sin escalar:
    el centro del lienzo de Pixellab va a la columna central de la celda y la
    fila más baja con píxeles de toda la animación (los pies) a la última fila.
    Los lienzos miden distinto según el estado (76..144 px), así que se ancla por
    el centro y los pies, nunca por la esquina."""
    ancho, alto = celda_wh
    pies = max(c.getchannel("A").getbbox()[3] for c in cuadros if c.getchannel("A").getbbox())
    celdas = []
    recortados = 0                                    # cuadros que se salen de la celda
    for c in cuadros:
        celda = Image.new("RGBA", (ancho, alto), (0, 0, 0, 0))
        x = (ancho - c.width) // 2                    # centro del lienzo -> centro de la celda
        y = alto - pies                               # pies -> última fila
        celda.paste(c, (x, y), c)                     # lo que quede fuera se recorta
        b = c.getchannel("A").getbbox()
        if b and (b[0] + x < 0 or b[2] + x > ancho or b[1] + y < 0):
            recortados += 1
        celdas.append(celda)
    if recortados:
        print(f"AVISO: {nombre or 'animación'}: {recortados} de {len(cuadros)} cuadros no caben "
              f"en {ancho}x{alto} y se recortan")
    return celdas


def cargar_pixellab(entrada: Path, regla: tuple, celda_wh: tuple = (CELDA_ANCHO, CELDA_ALTO),
                    nombre: str = "") -> list | None:
    """Cuadros de una animación del juego según su regla de MAPEOS_PIXELLAB, ya
    anclados en celdas. None si ese estado o dirección no está en la exportación."""
    estado, direccion, desde, hasta, al_reves = regla
    if desde is None:                                 # pose fija: rotations/<dirección>.png
        pose = entrada / estado / "rotations" / f"{direccion}.png"
        if not pose.is_file():
            return None
        with Image.open(pose) as im:
            return anclar([im.convert("RGBA")], celda_wh, nombre)
    # "Estado/animación" elige una animación concreta cuando el estado tiene varias
    # (al ninja se le añadieron bloqueo, agarre... al estado Fighting_stance).
    estado, _, animacion = estado.partition("/")
    patron = f"{animacion}/{direccion}" if animacion else f"*/{direccion}"
    carpetas = sorted((entrada / estado / "animations").glob(patron))
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
    return anclar(cuadros, celda_wh, nombre) if cuadros else None


# --- 2. Marcador numerado para las animaciones que faltan --------------------------

def marcador(animacion: str, k: int, total: int, celda_wh: tuple = (CELDA_ANCHO, CELDA_ALTO)) -> Image.Image:
    """Celda con un marco, el nombre de la animación y 'k/total'."""
    ancho, alto = celda_wh
    margen = (ancho - 56) // 2                        # marco de 56 px de ancho (20 px de margen en 96)
    im = Image.new("RGBA", (ancho, alto), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rectangle((margen, 4, ancho - margen - 1, alto - 1), fill=(70, 70, 150, 255), outline=(240, 240, 240, 255), width=2)
    fuente = ImageFont.load_default(size=9)
    lineas = animacion.split("_") + [f"{k + 1}/{total}"]
    for i, linea in enumerate(lineas):
        d.text((margen + 4, 10 + i * 13), linea, fill=(240, 240, 240, 255), font=fuente)
    return im


# --- 3. Normalizar cada cuadro a la celda --------------------------------------------

def a_celda(cuadro: Image.Image, celda_wh: tuple = (CELDA_ANCHO, CELDA_ALTO)) -> Image.Image:
    """Mete un cuadro en una celda de celda_wh: centrado y apoyado abajo si cabe,
    reducido (con pérdida, y avisando) solo si no cabe."""
    ancho, alto = celda_wh
    if cuadro.size == (ancho, alto):
        return cuadro
    if cuadro.width > ancho or cuadro.height > alto:
        factor = min(ancho / cuadro.width, alto / cuadro.height)
        cuadro = cuadro.resize((max(1, round(cuadro.width * factor)), max(1, round(cuadro.height * factor))), Image.NEAREST)
        print(f"AVISO: cuadro mayor que {ancho}x{alto}, reducido a {cuadro.size} (pierde detalle)")
    celda = Image.new("RGBA", (ancho, alto), (0, 0, 0, 0))
    celda.paste(cuadro, ((ancho - cuadro.width) // 2, alto - cuadro.height), cuadro)
    return celda


# --- 4. Paleta común y tiras indexadas ---------------------------------------------

def _distancia2(a: tuple, b: tuple) -> int:
    """Distancia al cuadrado entre dos colores RGB."""
    return (a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2 + (a[2] - b[2]) ** 2


def paleta_comun(cuadros: list, fijos: list = ()) -> list:
    """Hasta 15 colores opacos para TODOS los cuadros: exactos si ya son ≤ 15,
    cuantizados (median cut) si hay más.

    Con `fijos`, esos colores van PRIMERO y en su orden (índices 1..n de la
    paleta, siempre los mismos), y median cut reparte los 15 - n que quedan
    solo entre los píxeles que NINGÚN fijo cubre (a más de TOLERANCIA_FIJOS).
    Así un detalle pequeño (piel, acero) no pierde su color frente al traje."""
    # Pillow ≥ 12 llama get_flattened_data a lo que antes era getdata.
    datos = (lambda im: im.get_flattened_data() if hasattr(im, "get_flattened_data") else im.getdata())
    pixeles = [p[:3] for im in cuadros for p in datos(im) if p[3] > 0]
    unicos = sorted(set(pixeles))
    if len(unicos) <= COLORES:
        return unicos
    fijos = [tuple(c) for c in fijos]
    if len(fijos) > COLORES:
        sys.exit(f"ERROR: {len(fijos)} colores fijos y la paleta solo tiene {COLORES}.")
    if fijos:                                         # quitar lo que ya cubre un fijo
        tope = TOLERANCIA_FIJOS ** 2
        cubierto = {c: min(_distancia2(c, f) for f in fijos) <= tope for c in unicos}
        pixeles = [p for p in pixeles if not cubierto[p]]
    libres = COLORES - len(fijos)
    if libres == 0 or not pixeles:
        return fijos
    resto = sorted(set(pixeles))
    if len(resto) <= libres:                          # lo que queda ya cabe exacto
        return fijos + [c for c in resto if c not in fijos]
    fila = Image.new("RGB", (len(pixeles), 1))
    fila.putdata(pixeles)
    # Median cut a veces parte una zona grande (el negro del contorno) en tonos
    # casi idénticos: (0,0,0), (1,0,0), (0,0,1)... que gastan huecos sin que se
    # note la diferencia. Se unifican (gana el tono con más píxeles) y se vuelve
    # a cuantizar con un color más, hasta llenar los huecos libres.
    mejor = []
    for k in range(libres, libres + UNIFICAR_INTENTOS):
        q = fila.quantize(colors=k, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
        pal = q.getpalette()
        usos = sorted(q.getcolors(), reverse=True)    # (píxeles, índice), los más usados primero
        aceptados = []
        for _, i in usos:
            c = tuple(pal[i * 3: i * 3 + 3])
            if all(_distancia2(c, a) > UNIFICAR_DISTANCIA ** 2 for a in fijos + aceptados):
                aceptados.append(c)
        if len(aceptados) > libres:                   # este k ya se pasa: vale el anterior
            break
        if len(aceptados) > len(mejor):
            mejor = aceptados
        if len(mejor) == libres:
            break
    return fijos + sorted(mejor)


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

def escribir_header(ruta: Path, nombre: str, cuadros: dict, celda_wh: tuple = (CELDA_ANCHO, CELDA_ALTO)) -> None:
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
        f"      {N}_<ANIMACION>_CUADROS  1..{MAX_CUADROS}  cuadros de {celda_wh[0]}x{celda_wh[1]} de esa tira",
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
    """Prepara las animaciones de `nombre` (las de PERSONAJES[nombre], o las 14 de
    Tarou si no está). Devuelve {animación: cuadros}.
    No escribe nada si alguna animación no cumple el límite de cuadros."""
    config = PERSONAJES.get(nombre, POR_DEFECTO)
    celda_wh = config["celda"]
    CELDA_ANCHO, CELDA_ALTO = celda_wh                # sombrean las del módulo dentro de preparar
    por_animacion = {}
    nativo = es_pixellab_nativo(entrada)
    mapeo = MAPEOS_PIXELLAB.get(nombre, {})
    if nativo and not mapeo:
        sys.exit(f"ERROR: {entrada} es una exportación nativa de Pixellab pero no hay mapeo para '{nombre}' "
                 f"en MAPEOS_PIXELLAB. No se escribió nada.")
    for animacion, por_defecto in config["animaciones"]:
        if nativo:
            cuadros = cargar_pixellab(entrada, mapeo[animacion], celda_wh, animacion) if animacion in mapeo else None
            origen = "pixellab"
        else:
            cuadros = cargar_cuadros(entrada, animacion)
            origen = "arte"
        if cuadros is None:
            cuadros = [marcador(animacion, k, por_defecto, celda_wh) for k in range(por_defecto)]
            origen = "marcador"
        if len(cuadros) > MAX_CUADROS:
            sys.exit(f"ERROR: {animacion} tiene {len(cuadros)} cuadros y el máximo es {MAX_CUADROS} "
                     f"(cada animación debe caber en un banco de 32 KB). No se escribió nada.")
        por_animacion[animacion] = ([a_celda(c, celda_wh) for c in cuadros], origen)

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

    # La paleta sale del arte real; los marcadores solo cuentan si no hay nada más.
    # Si no, con muchos marcadores (Bárbara tiene 22) su azul y su blanco gastan
    # dos de los 15 colores; en el juego se dibujarán con el color más cercano.
    con_arte = [c for a, cuadros in alineados.items() if por_animacion[a][1] != "marcador" for c in cuadros]
    colores = paleta_comun(con_arte or [c for cuadros in alineados.values() for c in cuadros], config["fijos"])

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
    # Solo Tarou la lleva (config["estatico"]): Bárbara y los ninjas no tienen "reposo".
    if config["estatico"]:
        reposo = alineados["reposo"]
        estaticos = [reposo[(k * len(reposo)) // CUADROS_ESTATICOS] for k in range(CUADROS_ESTATICOS)]
        tira = Image.new("RGBA", (CELDA_ANCHO * CUADROS_ESTATICOS, CELDA_ALTO), (0, 0, 0, 0))
        for k, c in enumerate(estaticos):
            tira.paste(c, (k * CELDA_ANCHO, 0), c)
        indexar(tira, colores).save(salida / f"{nombre}_reposo_estatico.png", format="PNG")
        print(f"  {nombre}_reposo_estatico.png  {CUADROS_ESTATICOS} cuadros  (reposo {[(k * len(reposo)) // CUADROS_ESTATICOS for k in range(CUADROS_ESTATICOS)]})")
    escribir_header(salida / f"{nombre}_anim.h", nombre, resultado, celda_wh)
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
