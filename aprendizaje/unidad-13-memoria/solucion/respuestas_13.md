# Respuestas del ejercicio 13

Con la compilación normal del 2026-10-22 (rama `demo-secuencia-arranque`) y la
ROM de prueba `u13_memoria_guion.sfc`. Si tu código es otro, cambian los
números, no los razonamientos.

## Parte A

- **a)** `data.asm` tiene `nivel1_cerca_col_banco: .db :nivel1_cerca_col`. `:`
  es «el banco de» en WLA-DX. El **enlazador** (`wlalink`) escribe ese byte al
  colocar las secciones, cuando ya sabe en qué banco quedó `nivel1_cerca_col`
  (`$81`). Antes de enlazar nadie lo sabe: la sección es superfree.
- **b)** El C de tcc816 trabaja aquí con los 16 bits bajos: `(u16)&etiqueta` da
  `$B3BA`, y el banco de una sección superfree lo decide el enlazador después
  de compilar. No hay ninguna expresión de C que dé «el banco donde acabó esta
  etiqueta»; por eso se guarda en un byte de la ROM que el C lee como variable
  (`extern u8 nivel1_cerca_col_banco;`).
- **c)** En `escena_nintendo.asm`, la tabla es `ESCENA_NINTENDO dsb 64` dentro
  de una `RAMSECTION "ram{...}.data" APPENDTO "globram.data"` (la WRAM), y sus
  64 bytes de valores van en `.SECTION "{...}.data" APPENDTO "glob.data"` (la
  ROM). El arranque de la librería (`crt0_snes.asm`, bucle «copy .data
  section to RAM») copia `glob.data` a `globram.data` antes de `main`. Una `static const` (como `controles` de
  `pantalla_nivel1.c`) va en cambio en `.rodata`, en la ROM, sin copia.

## Parte B

1. Del `.log`: `ROM bank 15` a `ROM bank 127` tienen 32 768 bytes libres: **113
   bancos vacíos**, los bancos de la CPU **`$8F..$FF`**.
2. 32 768 / 3 072 = 10,67 → **10 cuadros por banco** (30 720 bytes; una
   sección no puede partirse). En los vacíos: 113 × 10 = **1 130 cuadros**.
3. `ROM bank 14` (`$8E`) tiene 10 723 bytes libres: 10 723 / 3 072 = 3,49 →
   **3 cuadros** más (si todo ese hueco es contiguo; el enlazador lo dirá).
   Total: **1 133 cuadros**, unas 113 animaciones de 10 cuadros.
4. La división ingenua: 3 713 537 / 3 072 = **1 208**. Sobra en ella todo lo que
   no se puede usar: los 2 048 bytes de cada banco que no llegan a un cuadro
   (113 × 2 048 = 231 424) y los bytes sueltos de los bancos llenos (0..10).
5. Ver A b): el DMA necesita el banco en `$4304` (`A1B0`) y la dirección de 16
   bits en `$4302`. La dirección no dice en qué banco está: `$B3BA` existe en
   los 128 bancos.

## Parte C

`tarou` en `$7E:2631` (18 bytes):

| Desplazamiento | Campo | Tipo |
|---|---|---|
| +0 | `x_fp` | `s16`, 12.4 (bajo, alto) |
| +2 | `y_fp` | `s16`, 12.4 |
| +4 | `vx_fp` | `s16`, 12.4 |
| +6 | `vy_fp` | `s16`, 12.4 |
| +8 | `estado` | `u8` (`enum Estado`: 0 PARADO, 1 CAMINANDO, 2 CORRIENDO, 3 SALTANDO, 4 CAYENDO, 5 AGACHADO...) |
| +9 | `espada` | `u8` |
| +10 | `direccion` | `u8` |
| +11 | `en_suelo` | `u8` (dirección `$7E:263C`) |
| +12 | `cuadro` | `u8` |
| +13 | `frames_cuadro` | `u8` |
| +14 | `anim_terminada` | `u8` |
| +15 | (relleno) | — |
| +16 | `frames_quieto` | `u16` |

Comprobación: si los dos primeros bytes son, por ejemplo, `40 1A`, `x_fp =
$1A40 = 6720` y `6720 / 16 = 420`: la `X` del HUD de la misma captura es 420.
`y_fp` en el suelo base: `SUELO_Y − ALTO_CELDA = 176 − 64 = 112` → `112 × 16 =
1792 = $0700` (bytes `00 07`).
