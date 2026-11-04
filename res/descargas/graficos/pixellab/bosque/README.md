# Pixellab → zona 2 de la fase 1: bosque de bambú

Deja aquí el arte de Pixellab de esta zona (git ignora todo lo que pongas aquí
salvo este README). Piezas y prompts:
[`docs/pixellab-fase1.md`](../../../../../docs/pixellab-fase1.md) §6.2; reglas
comunes de las zonas al principio de §6.

## Formato

- **Capa cercana (BG1, paleta 4):** tileset de 16×16 y piezas que se repiten.
  Trabajar con **256 tiles únicos de 8×8** por capa mientras no se decida el
  límite (§9).
- **Capa lejana (BG2, paleta 3):** **una imagen** ancha que se repite en
  horizontal (mapa de 256×256 px): el borde izquierdo y el derecho deben
  encajar.
- **15 colores + transparente** por paleta.
- **No se pide:** el degradado del cielo, las ondulaciones y franjas de scroll
  ni las luces por ventana: los hace el código con HDMA.

Aún no hay script que importe estas zonas; el precedente es el suelo del nivel
1 (`../suelo/` y `scripts/preparar_nivel.py`).
