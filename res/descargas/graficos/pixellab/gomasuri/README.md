# Pixellab → Gomasuri-dono (jefe de la fase 1)

Deja aquí la exportación de Pixellab de Gomasuri-dono (git ignora todo lo que
pongas aquí salvo este README). Prompts y cuadros:
[`docs/pixellab-fase1.md`](../../../../../docs/pixellab-fase1.md) §3.1.

**Antes del arte final: medirlo en la prueba de carga** (decidido el
2026-10-26). Un jefe lleno de 96×96 pasa de las 21 piezas del enemigo; si no
cabe, se pide más estrecho (64×96) o con menos piezas en la cabeza.

## Formato de la celda

- **Celda de 96×96**, figura de **90 px** con los pies en la última fila.
- Mirando a la **derecha**; **15 colores + transparente**, **paleta 6**, sin
  sombra ni suelo.
- **Máximo 6 cuadros por tira** en 96×96 (cada tira va en un banco de 32 KB).

## Nombres de archivo (propuestos)

`guardia` (4), `caminar` (6), `carga` (6), `tajo` (6, alabarda), `convocar`
(4), `recibir` (2), `derribado` (5), `levantarse` (6), `derrota` (8: pasa del
tope de 6 por tira, queda por decidir cómo se reparte), `entrada` (6).

El script de importación aún no conoce a este personaje ni la celda de 96×96.
