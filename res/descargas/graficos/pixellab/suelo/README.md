# Pixellab → suelo del nivel 1

Deja en **esta carpeta** la hoja de tiles exportada de Pixellab. Git ignora todo
lo que pongas aquí salvo este README. Después, desde la raíz del repositorio:

```
python scripts/preparar_suelo.py --seto res/titulo/fondo_cerca.png
```

El script toma el primer PNG de esta carpeta y escribe
`res/nivel1/fondo_cerca.png`, la capa cercana del nivel 1. Después, `make` (o
`run.bat`).

## Cómo se creó en Pixellab (2026-10-19)

1. **Create Tileset**, pestaña **Standard**, tipo **Sidescroller**.
2. Interior de la plataforma: `packed brown earth with small stones, simple
   shading`. Superficie superior: `short green grass edge, few tufts`.
3. **Tile size 16×16**, **transition size 0**, grosor de plataforma medio,
   detalle y sombreado bajos.
4. Exportar la hoja (se usó la de Godot; vale cualquiera que traiga los tiles
   de 16×16 en rejilla).

## Qué toma el script de la hoja

- **Borde**: tiles con la primera fila vacía y las 6 de abajo llenas (hierba
  sobre tierra). Usa la tira horizontal más larga, en su orden, y la repite.
- **Tierra**: tiles totalmente opacos y sin verde. Los reparte con volteos
  horizontales y verticales (semilla fija) para que no se vean franjas.
- La hierba empieza en la fila **176** (`SUELO_Y` del juego, `--suelo`).
- `--seto` pone detrás los arbustos del título, subidos para que asomen por
  encima del suelo. Sin la opción, el horizonte queda despejado.

## Reglas que impone la SNES

- **15 colores + transparente** en total, contando el seto (3 colores). El
  script reduce los de la tierra si hace falta.
- Fondo **transparente de verdad** (sin semitransparencias): por encima de la
  hierba se ven las montañas y el cielo.
- La capa cabe en unos 512 tiles de 8×8 (hoy usa ~243).
