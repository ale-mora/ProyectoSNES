---
description: Ordena CLAUDE.md (mide, propone destinos, mueve verbatim y verifica), con confirmación en cada etapa
---

Repite la reorganización de la memoria del repo del 2026-11-05. Las preguntas al usuario, siempre con el
selector (AskUserQuestion). No hagas commit ni push sin que lo pida.

1. **Medir** (sin escribir): `wc -c CLAUDE.md`, bytes por sección
   (`awk '/^###? /{if(t)printf "%6d  %s\n",c,t; t=$0; c=0} {c+=length($0)+1} END{printf "%6d  %s\n",c,t}' CLAUDE.md`),
   `git status` de CLAUDE.md (debe estar igual a HEAD: es el respaldo) y
   `python scripts/revisar_presupuesto_docs.py`. Muestra una tabla sección · bytes · acción propuesta.
2. **Plan**: clasifica cada bloque según la tabla «Dónde va cada cosa» de CLAUDE.md (trampas →
   `.claude/rules/<área>.md` con `paths:`; mediciones → `docs/<fecha>-<tema>.md`; estado y crónica →
   `docs/estado/*.md`; lo de toda sesión se queda). Presenta bloque → destino → `paths:` y espera la
   confirmación.
3. **Mover** con un script de Python (nunca reescribiendo a mano): texto verbatim, una nota de origen al
   principio de cada archivo destino y, en CLAUDE.md, una línea por regla + dónde está el detalle.
   Las contradicciones del original se reportan, no se resuelven en silencio.
4. **Verificar**: `git show HEAD:CLAUDE.md > orig.md` y comprueba que toda línea no vacía del original
   está en `CLAUDE.md`, `.claude/rules/*.md` o `docs/**/*.md`; reporta las que falten y por qué, los
   tamaños finales y `python scripts/revisar_presupuesto_docs.py`.
