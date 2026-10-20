#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    preparar_nivel.py — Convierte el nivel 1 dibujado en Tiled (la LÓGICA: qué es
    sólido, qué es zarza, dónde va cada cartel) en todo lo que necesita la ROM:
    la imagen de la capa cercana vestida con el arte de Pixellab, la rejilla de
    colisión y una cabecera con los objetos.

    PROPÓSITO
      El autor pinta en Tiled con un tileset de lógica de 3 tiles de 16x16
      (sólido, plataforma, zarza) y coloca objetos con nombre (inicio, cartel,
      tronco, salida). Este script:
        1. Lee el .tmj (JSON de Tiled) → rejilla de 128x14 celdas de 16 px.
        2. VISTE la rejilla con el tileset Sidescroller de Pixellab, que es de
           «rejilla doble» (dual grid): cada celda de arte cubre las esquinas de
           cuatro celdas de lógica y se elige por cuáles de ellas son sólidas
           (16 combinaciones). Las celdas de arte van desplazadas media celda
           (8 px) respecto de la lógica, así que todo cae en la rejilla de 8x8
           de la SNES y los tiles se repiten (pocos tiles distintos en VRAM).
        3. Pone detrás el seto (los arbustos del título), las zarzas y los
           carteles (tablero + letras de la fuente de consola).
        4. Escribe la rejilla de colisión (1 byte por celda) y una cabecera C
           con los objetos, que usan el juego y las pruebas del PC.
      Con --crear escribe además el .tmj INICIAL (el recorrido del tutorial,
      docs/diseno-tutorial-movimiento.md §2) y el tileset de lógica para Tiled.

    SE CONECTA CON
      Lee:      res/nivel1/tutorial.tmj (Tiled), el PNG más reciente de
                res/descargas/graficos/pixellab/suelo/, res/titulo/fondo_cerca.png
                (el seto) y res/fuente/ark_consola.bmp (letras de los carteles).
      Escribe:  res/nivel1/fondo_cerca.png (2048x256 indexada, capa cercana),
                res/nivel1/colision.bin (128x14 bytes), res/nivel1/nivel1_datos.h,
                y con --crear: res/nivel1/tutorial.tmj y res/nivel1/logica.png.
      Usa:      scripts/preparar_personaje.py (indexar) y
                scripts/preparar_suelo.py (reducir: median cut).
      Lo usan:  el Makefile (gfx4snes -m → tiles y mapa; mapa_a_columnas.py),
                data.asm (.rodata_nivel1_cerca, .rodata_nivel1_colision),
                src/juego/colision.c, src/pantallas/pantalla_nivel1.c y
                tests/ (la cabecera, con la rejilla para el PC).

    HARDWARE QUE TOCA
      ninguno; produce datos. Reglas de la PPU que respeta: índice 0 =
      transparente, 15 colores + transparente (una paleta de Mode 1), tiles
      de 8x8 alineados, y como mucho LIMITE_TILES tiles distintos (contando
      los volteos, que la SNES hace gratis): VRAM_FONDO_A admite 512.

    VARIABLES (constantes del módulo)
      CELDA         int  16 px    lado de una celda de lógica y de arte
      COLS, FILAS   int  128, 14  celdas del nivel (2048x224 px)
      ANCHO, ALTO   int  2048, 256  imagen de salida (224 visibles + relleno)
      VACIO, SOLIDO, PLATAFORMA, ZARZA  int 0..3  valores de la rejilla
      PERIODO_SETO  int  256 px   el seto se repite con este periodo (tiles)
      LIMITE_TILES  int  460      tope de tiles distintos (512 menos margen)

    Uso (desde la raíz del repo):
      python scripts/preparar_nivel.py --crear     # primera vez: escribe el .tmj
      python scripts/preparar_nivel.py             # tras retocar el .tmj en Tiled
    → glosario: Tile, Tilemap, Tileset, Metatile, Colisión, Punto de control
---------------------------------------------------------------------------------
"""
import argparse
import json
import random
import sys
from pathlib import Path

from PIL import Image, ImageDraw

sys.path.insert(0, str(Path(__file__).resolve().parent))
import preparar_personaje as pp  # noqa: E402  (indexar: color más cercano, índice 0 transparente)
import preparar_suelo as ps      # noqa: E402  (reducir: median cut)

CELDA = 16
COLS, FILAS = 128, 14
ANCHO, ALTO = COLS * CELDA, 256
VISIBLE = FILAS * CELDA
VACIO, SOLIDO, PLATAFORMA, ZARZA = 0, 1, 2, 3
PERIODO_SETO = 256
LIMITE_TILES = 460
SEMILLA = 2026

CARPETA_PIXELLAB = Path("res/descargas/graficos/pixellab/suelo")
TMJ = Path("res/nivel1/tutorial.tmj")
LOGICA_PNG = Path("res/nivel1/logica.png")
SETO = Path("res/titulo/fondo_cerca.png")
FUENTE = Path("res/fuente/ark_consola.bmp")
SALIDA_PNG = Path("res/nivel1/fondo_cerca.png")
SALIDA_BIN = Path("res/nivel1/colision.bin")
SALIDA_H = Path("res/nivel1/nivel1_datos.h")


def pixeles(im: Image.Image):
    """Los píxeles de una imagen (get_flattened_data en Pillow nuevo, getdata en el viejo)."""
    return im.get_flattened_data() if hasattr(im, "get_flattened_data") else im.getdata()


# --- 1. El recorrido inicial (solo con --crear) -------------------------------------

def trazado_inicial():
    """Rejilla y objetos del tutorial de docs/diseno-tutorial-movimiento.md §2.
    Coordenadas en celdas de 16 px; el suelo base ocupa las filas 11..13 (y 176..223)."""
    g = [[VACIO] * COLS for _ in range(FILAS)]

    def rect(c0, c1, f0, f1, v=SOLIDO):              # columnas y filas inclusivas
        for f in range(f0, f1 + 1):
            for c in range(c0, c1 + 1):
                g[f][c] = v

    rect(0, COLS - 1, 11, 13)                         # suelo base
    rect(21, 23, 10, 10)                              # 2. escalón de 16 px
    rect(24, 26, 9, 10)                               #    y escalón de 32 px
    rect(37, 39, 8, 10)                               # 3. muro de 48 px
    rect(53, 58, 0, 7)                                # 4. techo del túnel: hueco de 48 px
    rect(70, 75, 11, 13, VACIO)                       # 5. foso de 96 px (6 celdas)
    rect(85, 92, 10, 10, ZARZA)                       # 6. zarzas sobre el suelo
    rect(85, 87, 8, 8, PLATAFORMA)                    #    con dos plataformas encima
    rect(89, 91, 8, 8, PLATAFORMA)
    objetos = [
        {"nombre": "inicio", "x": 88},
        {"nombre": "cartel", "x": 104, "texto": "WALK|< >"},
        {"nombre": "cartel", "x": 296, "texto": "JUMP|B"},
        {"nombre": "cartel", "x": 536, "texto": "HOLD B|HIGHER"},
        {"nombre": "cartel", "x": 792, "texto": "CROUCH|DOWN"},
        {"nombre": "cartel", "x": 1048, "texto": "HOLD Y|RUN"},
        {"nombre": "cartel", "x": 1304, "texto": "MIND|THORNS"},
        {"nombre": "cartel", "x": 1560, "texto": "R SWORD|A HIT"},
        {"nombre": "tronco", "x": 1640},
        {"nombre": "cartel", "x": 1816, "texto": "EXIT|>>"},
        {"nombre": "salida", "x": 1968},
    ]
    return g, objetos


def escribir_logica_png(ruta: Path) -> None:
    """Tileset de lógica para Tiled: sólido (marrón), plataforma (azul), zarza (rojo)."""
    im = Image.new("RGBA", (3 * CELDA, CELDA), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for i, (color, letra) in enumerate([((140, 90, 50, 255), "#"), ((60, 120, 220, 255), "="),
                                        ((210, 40, 40, 255), "^")]):
        d.rectangle([i * CELDA, 0, i * CELDA + CELDA - 1, CELDA - 1], fill=color, outline=(0, 0, 0, 255))
        d.text((i * CELDA + 5, 2), letra, fill=(255, 255, 255, 255))
    ruta.parent.mkdir(parents=True, exist_ok=True)
    im.save(ruta)


def escribir_tmj(ruta: Path, g, objetos) -> None:
    """Mapa de Tiled en JSON. OJO: sin espacios tras los dos puntos (separators), como
    exporta Tiled: cute_tiled (tmx2snes) falla con ellos y así el .tmj es idéntico al
    que guardaría Tiled."""
    datos = [g[f][c] for f in range(FILAS) for c in range(COLS)]  # gid = valor (0 = vacío)
    objs = []
    for i, o in enumerate(objetos, start=1):
        ob = {"id": i, "name": o["nombre"], "type": "", "point": True, "rotation": 0,
              "visible": True, "x": o["x"], "y": 176, "width": 0, "height": 0}
        if "texto" in o:
            ob["properties"] = [{"name": "texto", "type": "string", "value": o["texto"]}]
        objs.append(ob)
    mapa = {
        "compressionlevel": -1, "height": FILAS, "width": COLS, "infinite": False,
        "orientation": "orthogonal", "renderorder": "right-down", "tiledversion": "1.11.0",
        "tilewidth": CELDA, "tileheight": CELDA, "type": "map", "version": "1.10",
        "nextlayerid": 3, "nextobjectid": len(objs) + 1,
        "layers": [
            {"id": 1, "name": "logica", "type": "tilelayer", "x": 0, "y": 0, "width": COLS,
             "height": FILAS, "opacity": 1, "visible": True, "data": datos},
            {"id": 2, "name": "objetos", "type": "objectgroup", "draworder": "topdown",
             "x": 0, "y": 0, "opacity": 1, "visible": True, "objects": objs},
        ],
        "tilesets": [{
            "firstgid": 1, "name": "logica", "image": "logica.png", "imagewidth": 3 * CELDA,
            "imageheight": CELDA, "tilewidth": CELDA, "tileheight": CELDA, "tilecount": 3,
            "columns": 3, "margin": 0, "spacing": 0,
        }],
    }
    ruta.parent.mkdir(parents=True, exist_ok=True)
    ruta.write_text(json.dumps(mapa, indent=1, separators=(",", ":")), encoding="utf-8")


# --- 2. Leer el .tmj -------------------------------------------------------------------

def leer_tmj(ruta: Path):
    """Devuelve (rejilla FILAS x COLS, objetos). Falla con un mensaje claro si el mapa
    no tiene el tamaño o la codificación esperados."""
    mapa = json.loads(ruta.read_text(encoding="utf-8"))
    if (mapa.get("width"), mapa.get("height"), mapa.get("tilewidth")) != (COLS, FILAS, CELDA):
        sys.exit(f"ERROR: {ruta} debe medir {COLS}x{FILAS} tiles de {CELDA} px.")
    capa = next((l for l in mapa["layers"] if l.get("type") == "tilelayer"), None)
    if capa is None or not isinstance(capa.get("data"), list):
        sys.exit(f"ERROR: {ruta} no tiene una capa de tiles en CSV (en Tiled: Map → Map "
                 "Properties → Tile Layer Format = CSV).")
    primero = mapa["tilesets"][0]["firstgid"]
    g = [[VACIO] * COLS for _ in range(FILAS)]
    for i, gid in enumerate(capa["data"]):
        gid &= 0x1FFFFFFF                              # quita los bits de volteo de Tiled
        if gid:
            v = gid - primero + 1                      # 1 sólido, 2 plataforma, 3 zarza
            g[i // COLS][i % COLS] = v if v in (SOLIDO, PLATAFORMA, ZARZA) else VACIO
    objetos = []
    for l in mapa["layers"]:
        if l.get("type") != "objectgroup":
            continue
        for o in l.get("objects", []):
            ob = {"nombre": o.get("name", ""), "x": int(o.get("x", 0))}
            for p in o.get("properties", []):
                if p.get("name") == "texto":
                    ob["texto"] = str(p.get("value", ""))
            objetos.append(ob)
    return g, objetos


# --- 3. Vestir la lógica con el arte de Pixellab (rejilla doble) --------------------

ESQUINAS = [(3, 3), (12, 3), (3, 12), (12, 12)]      # TL, TR, BL, BR: dónde se mira el alfa


def piezas_por_esquinas(hoja: Image.Image) -> dict:
    """Clave de 4 bits (TL=1, TR=2, BL=4, BR=8: esquina con tierra) → celdas de 16x16."""
    a = hoja.getchannel("A").load()
    piezas = {}
    for f in range(hoja.height // CELDA):
        for c in range(hoja.width // CELDA):
            k = sum(1 << b for b, (x, y) in enumerate(ESQUINAS) if a[c * CELDA + x, f * CELDA + y])
            piezas.setdefault(k, []).append(hoja.crop((c * CELDA, f * CELDA, (c + 1) * CELDA, (f + 1) * CELDA)))
    for k, sustituta in ((6, 14), (9, 13)):           # diagonales: la hoja no las trae
        if k not in piezas and sustituta in piezas:
            piezas[k] = piezas[sustituta]
    faltan = [k for k in range(16) if k not in piezas]
    if faltan:
        sys.exit(f"ERROR: a la hoja de Pixellab le faltan las combinaciones {faltan}; "
                 "¿es una exportación Sidescroller con transiciones?")
    return piezas


def tierra_en(g, c: int, f: int) -> bool:
    """¿Lleva arte de tierra la celda? Fuera del mapa: los lados repiten el borde y
    abajo repite la última fila (los fosos siguen hacia abajo); arriba, vacío."""
    if f < 0:
        return False
    c = min(max(c, 0), COLS - 1)
    f = min(f, FILAS - 1)
    return g[f][c] in (SOLIDO, PLATAFORMA)


def vestir(capa: Image.Image, g, piezas: dict) -> None:
    """Pega una celda de arte en cada vértice de la rejilla de lógica."""
    azar = random.Random(SEMILLA)
    volteos = [lambda t: t, lambda t: t.transpose(Image.FLIP_LEFT_RIGHT),
               lambda t: t.transpose(Image.FLIP_TOP_BOTTOM), lambda t: t.transpose(Image.ROTATE_180)]
    for f in range(FILAS + 1):
        for c in range(COLS + 1):
            k = (tierra_en(g, c - 1, f - 1) | tierra_en(g, c, f - 1) << 1
                 | tierra_en(g, c - 1, f) << 2 | tierra_en(g, c, f) << 3)
            if k == 0:
                continue
            if k == 15:                                # tierra: variantes y volteos al azar
                t = azar.choice(volteos)(azar.choice(piezas[15]))
            else:                                      # bordes: variantes en ciclo de 4 (se repiten)
                v = piezas[k][:4]
                t = v[c % len(v)]
            capa.alpha_composite(t, (c * CELDA - CELDA // 2, f * CELDA - CELDA // 2))


# --- 4. Seto, zarzas y carteles -------------------------------------------------------

def cargar_seto() -> Image.Image:
    """Los arbustos del título (con su alfa desde los índices: el 0 es transparente),
    recortados a su caja y a PERIODO_SETO de ancho."""
    with Image.open(SETO) as s:
        arb = s.convert("RGBA")
        indices = Image.frombytes("L", s.size, s.tobytes())
        arb.putalpha(Image.eval(indices, lambda i: 255 if i else 0))
    caja = arb.getchannel("A").getbbox()
    return arb.crop((0, caja[1], PERIODO_SETO, caja[3]))


def poner_seto(capa: Image.Image, g, seto: Image.Image) -> None:
    """El seto va detrás, con la base 16 px por debajo de la superficie del suelo base
    (fila 11 = y 176): la tierra lo tapa y solo asoman las copas. Se omite sobre los
    fosos (columnas sin suelo base)."""
    base = 11 * CELDA + CELDA
    for x0 in range(0, ANCHO, PERIODO_SETO):
        trozo = seto.copy()
        for c in range(PERIODO_SETO // CELDA):         # borra el seto encima de los fosos
            col = x0 // CELDA + c
            if col < COLS and g[11][col] == VACIO:
                ImageDraw.Draw(trozo).rectangle([c * CELDA, 0, c * CELDA + CELDA - 1, trozo.height], fill=(0, 0, 0, 0))
        capa.alpha_composite(trozo, (x0, base - trozo.height))


def poner_zarzas(capa: Image.Image, g, oscuro, claro) -> None:
    """Marcador de zarza (hasta que llegue el arte): púas sobre una mata, 16x16."""
    t = Image.new("RGBA", (CELDA, CELDA), (0, 0, 0, 0))
    d = ImageDraw.Draw(t)
    d.rectangle([0, 10, 15, 15], fill=oscuro + (255,))
    for x in (1, 6, 11):
        d.polygon([(x, 10), (x + 2, 2), (x + 4, 10)], fill=oscuro + (255,))
        d.point((x + 2, 3), fill=claro + (255,))
    for f in range(FILAS):
        for c in range(COLS):
            if g[f][c] == ZARZA:
                capa.alpha_composite(t, (c * CELDA, f * CELDA))


def suelo_bajo(g, col: int) -> int:
    """Fila de píxel donde empieza lo sólido bajo la columna (o el borde de abajo)."""
    for f in range(FILAS):
        if g[f][col] in (SOLIDO, PLATAFORMA):
            return f * CELDA
    return VISIBLE


def poner_cartel(capa: Image.Image, g, x: int, texto: str, fuente: Image.Image, colores) -> None:
    """Tablero con marco de 1 tile y las letras en tiles de 8x8 alineados: la misma
    letra en dos carteles es el MISMO tile (se comparten en VRAM). Poste debajo."""
    marco, fondo, letra = colores
    lineas = [l.upper() for l in texto.split("|")][:2]
    ancho_t = max(len(l) for l in lineas) + 2          # en tiles, con el marco
    alto_t = len(lineas) + 2
    w, h = ancho_t * 8, alto_t * 8
    x0 = (x - w // 2) // 8 * 8                         # alineado a 8
    suelo = suelo_bajo(g, min(max(x // CELDA, 0), COLS - 1))
    y0 = (suelo - 16 - h) // 8 * 8                     # 16 px de poste
    d = ImageDraw.Draw(capa)
    poste_x = x0 + (ancho_t // 2) * 8 + 2
    d.rectangle([poste_x, y0 + h, poste_x + 3, suelo + 3], fill=marco + (255,))
    d.rectangle([x0, y0, x0 + w - 1, y0 + h - 1], fill=fondo + (255,), outline=marco + (255,))
    fpx = fuente.load()
    for i, linea in enumerate(lineas):
        xi = x0 + 8 + (ancho_t - 2 - len(linea)) // 2 * 8
        for j, ch in enumerate(linea):
            k = ord(ch) - 32
            if not 0 <= k < 96:
                continue
            for yy in range(8):
                for xx in range(8):
                    if fpx[k * 8 + xx, yy]:
                        capa.putpixel((xi + j * 8 + xx, y0 + 8 + i * 8 + yy), letra + (255,))


# --- 5. Contar tiles como lo hará gfx4snes ---------------------------------------------

def tiles_distintos(im: Image.Image) -> int:
    """Tiles de 8x8 distintos de una imagen P, contando como iguales los volteados."""
    vistos = set()
    b = im.tobytes()
    for ty in range(im.height // 8):
        for tx in range(im.width // 8):
            filas = [b[(ty * 8 + y) * im.width + tx * 8: (ty * 8 + y) * im.width + tx * 8 + 8] for y in range(8)]
            v = [tuple(filas), tuple(r[::-1] for r in filas), tuple(filas[::-1]), tuple(r[::-1] for r in filas[::-1])]
            vistos.add(min(v))
    return len(vistos)


# --- 6. Salidas --------------------------------------------------------------------------

def escribir_cabecera(ruta: Path, g, objetos) -> None:
    def x_de(nombre, defecto):
        return next((o["x"] for o in objetos if o["nombre"] == nombre), defecto)
    carteles = sorted(o["x"] for o in objetos if o["nombre"] == "cartel")
    lineas = [
        "/* GENERADO por scripts/preparar_nivel.py desde res/nivel1/tutorial.tmj: no editar. */",
        "#ifndef NIVEL1_DATOS_H",
        "#define NIVEL1_DATOS_H",
        "",
        f"#define NIVEL1_COLS {COLS}            /* celdas de 16 px de ancho              */",
        f"#define NIVEL1_FILAS {FILAS}            /* celdas de 16 px de alto               */",
        f"#define NIVEL1_INICIO_X {x_de('inicio', 88)}      /* centro de Tarou al empezar (px)       */",
        f"#define NIVEL1_TRONCO_X {x_de('tronco', 0)}    /* borde izquierdo del tronco (px)       */",
        f"#define NIVEL1_SALIDA_X {x_de('salida', ANCHO - 64)}    /* tocarla termina el nivel (px)         */",
        f"#define NIVEL1_CONTROLES {len(carteles)}        /* puntos de control (uno por cartel)    */",
        "",
        "/* x (px) de cada punto de control, de izquierda a derecha. */",
        "#define NIVEL1_CONTROLES_X { " + ", ".join(str(x) for x in carteles) + " }",
        "",
        "#ifdef PRUEBAS_PC",
        "/* En el PC la rejilla va aquí; en la ROM la pone data.asm (colision.bin). */",
        "static const unsigned char nivel1_colision[NIVEL1_COLS * NIVEL1_FILAS] = {",
    ]
    for f in range(FILAS):
        lineas.append("    " + ",".join(str(v) for v in g[f]) + ",")
    lineas += ["};", "#endif", "", "#endif", ""]
    ruta.write_text("\n".join(lineas), encoding="utf-8")


def preparar(tmj: Path, hoja: Path, salida_png: Path = SALIDA_PNG, salida_bin: Path = SALIDA_BIN,
             salida_h: Path = SALIDA_H, limite: int = LIMITE_TILES) -> int:
    g, objetos = leer_tmj(tmj)
    with Image.open(hoja) as im:
        piezas = piezas_por_esquinas(im.convert("RGBA"))
    capa = Image.new("RGBA", (ANCHO, ALTO), (0, 0, 0, 0))
    seto = cargar_seto()
    poner_seto(capa, g, seto)
    vestir(capa, g, piezas)
    # Paleta: el seto conserva sus colores; el suelo se reduce a lo que quede de 15.
    colores_seto = sorted({p[:3] for p in pixeles(seto) if p[3] > 0})
    suelo = [p[:3] for t in piezas.values() for c in t for p in pixeles(c) if p[3] > 0]
    colores = colores_seto + [c for c in ps.reducir(suelo, pp.COLORES - len(colores_seto)) if c not in colores_seto]
    por_luz = sorted(colores, key=lambda c: c[0] * 3 + c[1] * 6 + c[2])
    verdes = sorted([c for c in colores if c[1] > c[0] + 20], key=lambda c: sum(c))
    oscuro, claro = por_luz[0], por_luz[-1]
    poner_zarzas(capa, g, verdes[0] if verdes else oscuro, claro)
    with Image.open(FUENTE) as f:
        fuente = f.convert("L")
    pardos = [c for c in colores if c[0] > c[1] > c[2]]
    fondo_cartel = sorted(pardos, key=sum)[len(pardos) // 2] if pardos else por_luz[len(por_luz) // 2]
    for o in objetos:
        if o["nombre"] == "cartel":
            poner_cartel(capa, g, o["x"], o.get("texto", ""), fuente, (oscuro, fondo_cartel, claro))
    final = pp.indexar(capa, colores)
    n = tiles_distintos(final)
    if n > limite:
        sys.exit(f"ERROR: la capa cercana tendría {n} tiles distintos (máximo {limite}). "
                 "Quita carteles o variedad de arte. No se escribió nada.")
    salida_png.parent.mkdir(parents=True, exist_ok=True)
    final.save(salida_png, format="PNG")
    salida_bin.write_bytes(bytes(v for fila in g for v in fila))
    escribir_cabecera(salida_h, g, objetos)
    print(f"  {salida_png}  {ANCHO}x{ALTO}; {n} tiles distintos; {len(colores)} colores; "
          f"{sum(1 for o in objetos if o['nombre'] == 'cartel')} carteles")
    print(f"  {salida_bin}  {COLS}x{FILAS} celdas;  {salida_h}")
    return n


def main() -> int:
    ap = argparse.ArgumentParser(description="Nivel 1: de Tiled (lógica) a capa cercana + colisión.")
    ap.add_argument("--crear", action="store_true", help="escribe el .tmj inicial y el tileset de lógica")
    ap.add_argument("--tmj", type=Path, default=TMJ)
    args = ap.parse_args()
    if args.crear:
        g, objetos = trazado_inicial()
        escribir_tmj(args.tmj, g, objetos)
        escribir_logica_png(LOGICA_PNG)
        print(f"  {args.tmj} y {LOGICA_PNG} creados (ábrelos con Tiled)")
    pngs = sorted(CARPETA_PIXELLAB.glob("*.png"))
    if not pngs:
        sys.exit(f"ERROR: no hay ningún PNG en {CARPETA_PIXELLAB}")
    hoja = max(pngs, key=lambda p: p.stat().st_mtime)
    print(f"[nivel] mapa: {args.tmj}; hoja: {hoja}")
    preparar(args.tmj, hoja)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
