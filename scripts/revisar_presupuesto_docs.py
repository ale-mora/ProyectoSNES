#!/usr/bin/env python3
"""
---------------------------------------------------------------------------------
    revisar_presupuesto_docs.py — El freno contra el recrecimiento de CLAUDE.md.

    PROPÓSITO
      CLAUDE.md se carga ENTERO en cada sesión de Claude Code y las reglas de
      .claude/rules/ se cargan solas al tocar los archivos de su `paths:`. Este script
      comprueba los topes: CLAUDE.md hasta 40.000 bytes, cada regla hasta 50.000, y que
      toda regla declare `paths:` (sin él se cargaría en TODAS las sesiones). Avisa al
      90 %.
        python scripts/revisar_presupuesto_docs.py          a mano: sale 1 si se pasa
        python scripts/revisar_presupuesto_docs.py --hook   hook PostToolUse de Claude
                                                            Code (.claude/settings.json):
                                                            sale 2 si se pasa
      En modo hook lee de la entrada estándar el JSON de la herramienta y solo revisa si
      se editó CLAUDE.md o una regla.

    SE CONECTA CON
      Lo usan:  .claude/settings.json (hook), la sección «Dónde va cada cosa» de CLAUDE.md.

    HARDWARE QUE TOCA
      ninguno.

    VARIABLES: TOPE_CLAUDE, TOPE_REGLA, AVISO.
---------------------------------------------------------------------------------
"""
import json
import os
import re
import sys
from pathlib import Path

RAIZ = Path(os.environ.get("CLAUDE_PROJECT_DIR") or Path(__file__).resolve().parents[1])
TOPE_CLAUDE = 40_000
TOPE_REGLA = 50_000
AVISO = 0.9

ARREGLO = """
Cómo corregirlo (CLAUDE.md, «Dónde va cada cosa»):
- CLAUDE.md: la regla en UNA línea + dónde está el detalle.
- Detalle de una trampa -> la regla de .claude/rules/ cuyo paths: cubre ese código.
- Mediciones y forenses -> docs/<fecha>-<tema>.md.
- Estado del juego, crónica, pendientes -> docs/estado/*.md.
- Mover el texto VERBATIM; no borrar información."""


def revisar(raiz=RAIZ):
    errores, avisos = [], []
    claude = raiz / "CLAUDE.md"
    if claude.exists():
        n = len(claude.read_bytes())
        if n > TOPE_CLAUDE:
            errores.append(f"CLAUDE.md pesa {n} bytes (tope {TOPE_CLAUDE}).")
        elif n > TOPE_CLAUDE * AVISO:
            avisos.append(f"CLAUDE.md está en {n} de {TOPE_CLAUDE} bytes.")
    reglas = raiz / ".claude" / "rules"
    if reglas.exists():
        for f in sorted(reglas.glob("*.md")):
            datos = f.read_bytes()
            n = len(datos)
            if n > TOPE_REGLA:
                errores.append(f".claude/rules/{f.name} pesa {n} bytes (tope {TOPE_REGLA}): pártela por tema.")
            elif n > TOPE_REGLA * AVISO:
                avisos.append(f".claude/rules/{f.name} está en {n} de {TOPE_REGLA} bytes.")
            fm = re.match(r"^---\r?\n(.*?)\r?\n---", datos.decode("utf-8", "replace"), re.S)
            if not fm or not re.search(r"^paths:", fm.group(1), re.M):
                errores.append(f".claude/rules/{f.name} no declara `paths:` => se cargaría en TODAS las sesiones.")
    return errores, avisos


def main(argv):
    if "--hook" in argv:
        try:
            datos = json.loads(sys.stdin.read() or "{}")
        except ValueError:
            return 0
        entrada = datos.get("tool_input") or {}
        ruta = str(entrada.get("file_path") or entrada.get("path") or "").replace("\\", "/")
        if not (re.search(r"(^|/)CLAUDE\.md$", ruta, re.I) or re.search(r"/\.claude/rules/[^/]+\.md$", ruta, re.I)):
            return 0
        errores, avisos = revisar()
        if errores:
            sys.stderr.write("Presupuesto de documentación excedido:\n- " + "\n- ".join(errores) + "\n" + ARREGLO + "\n")
            return 2
        if avisos:
            sys.stderr.write(" ".join(avisos) + "\n")
        return 0
    errores, avisos = revisar()
    for a in avisos:
        print(a, file=sys.stderr)
    if errores:
        for e in errores:
            print(e, file=sys.stderr)
        print(ARREGLO, file=sys.stderr)
        return 1
    print("Presupuesto de documentación OK.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
